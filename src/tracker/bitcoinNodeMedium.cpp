// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "bitcoinNodeMedium.h"
#include "scanBitcoinNetworkRequest.h"


namespace tracker
{

CBitcoinNodeMedium::CBitcoinNodeMedium( CNode * _node )
	: m_node( _node )
{
}

bool
CBitcoinNodeMedium::serviced() const
{
	return true;
}

bool
CBitcoinNodeMedium::flush()
{
// not sure if it is ok
	return true;

}

bool
CBitcoinNodeMedium::getResponse( std::vector< TrackerResponses > & _requestResponse ) const
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	if ( m_merkles.empty() )
		_requestResponse.push_back( common::CContinueResult( 0 ) );
	else
	{
		_requestResponse = m_responses;
	}

	return true;
}

void
CBitcoinNodeMedium::clearResponses()
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_responses.clear();
	m_merkles.clear();
	m_transactions.clear();
}


void
CBitcoinNodeMedium::add( CAskForTransactionsRequest const * _request )
{
	boost::lock_guard<boost::mutex> lock( m_node->m_mediumLock );

	m_node->m_filterSendQueue.push_back( CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_P2PUBKEY_ONLY) );
	m_node->m_blockQueue.insert( m_node->m_blockQueue.end(), _request->getBlockHashes().begin(), _request->getBlockHashes().end() );
}

void
CBitcoinNodeMedium::add( CSetBloomFilterRequest const * _request )
{
	boost::lock_guard<boost::mutex> lock( m_node->m_mediumLock );
	m_node->m_filterSendQueue.push_back( _request->getBloomFilter() );

}

void
CBitcoinNodeMedium::reloadResponses()
{
	m_responses.clear();
	m_responses.push_back( CRequestedMerkles( m_merkles, m_transactions,reinterpret_cast< long long >( m_node ) ) );
}
void
CBitcoinNodeMedium::setResponse( CMerkleBlock const & _merkle )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_merkles.push_back( _merkle );
	reloadResponses();
}
// it is tricky  because for simplification tx and merkle have to go through the same channel in concurrent way
void
CBitcoinNodeMedium::setResponse( CTransaction const & _tx )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_transactions.push_back( _tx );
	reloadResponses();
}

}
