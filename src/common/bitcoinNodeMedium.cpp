// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/bitcoinNodeMedium.h"

namespace common
{

CBitcoinNodeMedium ::CBitcoinNodeMedium( CNode * _node )
	: m_node( _node )
{
}

bool
CBitcoinNodeMedium ::serviced() const
{
	return !m_responses.empty();//!m_transactions.empty() || !m_merkles.empty();
}

bool
CBitcoinNodeMedium ::flush()
{
// not sure if it is ok
	return true;

}

bool
CBitcoinNodeMedium ::getResponseAndClear( std::multimap< common::CRequest const*, DimsResponse > & _requestResponse )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	_requestResponse = m_responses;

	clearResponses();
	return true;
}

void
CBitcoinNodeMedium ::clearResponses()
{
	m_responses.clear();
	m_merkles.clear();
	m_transactions.clear();
}

void
CBitcoinNodeMedium ::add( CAskForTransactionsRequest  const * _request )
{
	LOCK( m_node->m_mediumLock );
	LastRequest = (common::CRequest *)_request;

	CBloomFilter bloomFilter =  CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_P2PUBKEY_ONLY);

	bloomFilter.insert(Params().getOriginAddressKeyId());

	m_node->m_filterSendQueue.push_back( bloomFilter );
	m_node->m_blockQueue.insert( m_node->m_blockQueue.end(), _request->getBlockHashes().begin(), _request->getBlockHashes().end() );

	reloadResponses();
}

void
CBitcoinNodeMedium ::add( CSetBloomFilterRequest  const * _request )
{
	LOCK( m_node->m_mediumLock );
	LastRequest = (common::CRequest *)_request;
	m_node->m_filterSendQueue.push_back( _request->getBloomFilter() );

}

void
CBitcoinNodeMedium ::reloadResponses()
{
	m_responses.clear();

	m_responses.insert( std::make_pair( LastRequest, CRequestedMerkles( m_merkles, m_transactions,reinterpret_cast< uintptr_t >( m_node ) ) ) );
}

void
CBitcoinNodeMedium ::setResponse( CMerkleBlock const & _merkle )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_merkles.push_back( _merkle );
	reloadResponses();
}
// it is tricky  because for simplification tx and merkle have to go through the same channel in concurrent way
void
CBitcoinNodeMedium ::setResponse( CTransaction const & _tx )
{
// using m_merkle.back may cause  problem, when merkle  will be  processed but tx will not  arrive on time
//m_merkle may be  cleared ?? is this  really a potential problem ??
// if  so  I have to remember last processed  hash
	boost::lock_guard<boost::mutex> lock( m_mutex );

	if ( m_merkles.empty() )
		return;

	std::vector<uint256> match;
	m_merkles.back().txn.ExtractMatches( match );

	if ( std::find( match.begin(), match.end(), _tx.GetHash() ) == match.end() )
			return;

	uint256 hash = m_merkles.back().header.GetHash();

	if( m_transactions.find( hash ) == m_transactions.end() )
	{
		std::vector< CTransaction > transactions;
		transactions.push_back( _tx );
		m_transactions.insert( std::make_pair( hash, transactions ) );
	}
	else
	{
		m_transactions.find( hash )->second.push_back( _tx );
	}

	reloadResponses();
}

}
