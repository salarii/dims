#ifndef BITCOIN_NODE_MEDIUM_H
#define BITCOIN_NODE_MEDIUM_H

#include "common/types.h"
#include "common/medium.h"
#include "common/commonRequests.h"

#include "bitcoinNodeMedium.h"
#include "chainparams.h"
#include "main.h"

class CNode;

namespace common
{

template < class _Types >
class CAskForTransactionsRequest;

template < class _Types >
class CSetBloomFilterRequest;

// current  implementation does not  pay  attention  to checking  if  responses are correctly assigned   to correspondent  request( I don't know if this may cause problems )
// besides this, it is primitie and ugly but  still may  serve
template < class _Type >
class CBitcoinNodeMedium : public _Type::Medium
{
public:
	CBitcoinNodeMedium( CNode * _node );

	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::multimap< common::CRequest< _Type >const*, typename _Type::Response > & _requestResponse );

	virtual void add( CAskForTransactionsRequest< _Type > const * _request );

	virtual void add( CSetBloomFilterRequest< _Type > const * _request );

	void setResponse( CTransaction const & _tx );

	void setResponse( CMerkleBlock const & _merkle );
private:
	void clearResponses();

	void reloadResponses();
private:
	mutable boost::mutex m_mutex;

	std::multimap< common::CRequest< _Type >const*, typename _Type::Response > m_responses;

	std::map< uint256 ,std::vector< CTransaction > > m_transactions;

	std::vector< CMerkleBlock > m_merkles;

	CNode * m_node;

	common::CRequest< _Type > * LastRequest;//ugly way  to do responses <-> request  matching
};



template < class _Type >
CBitcoinNodeMedium< _Type >::CBitcoinNodeMedium( CNode * _node )
	: m_node( _node )
{
}

template < class _Type >
bool
CBitcoinNodeMedium< _Type >::serviced() const
{
	return !m_transactions.empty() || !m_merkles.empty();
}

template < class _Type >
bool
CBitcoinNodeMedium< _Type >::flush()
{
// not sure if it is ok
	return true;

}

template < class _Type >
bool
CBitcoinNodeMedium< _Type >::getResponseAndClear( std::multimap< common::CRequest< _Type >const*, typename _Type::Response > & _requestResponse )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	_requestResponse = m_responses;

	clearResponses();
	return true;
}

template < class _Type >
void
CBitcoinNodeMedium< _Type >::clearResponses()
{
	m_responses.clear();
	m_merkles.clear();
	m_transactions.clear();
}

template < class _Type >
void
CBitcoinNodeMedium< _Type >::add( CAskForTransactionsRequest< _Type > const * _request )
{
	LOCK( m_node->m_mediumLock );
	LastRequest = (common::CRequest< _Type >*)_request;

	CBloomFilter bloomFilter =  CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_P2PUBKEY_ONLY);

	bloomFilter.insert(Params().getOriginAddressKeyId());

	m_node->m_filterSendQueue.push_back( bloomFilter );
	m_node->m_blockQueue.insert( m_node->m_blockQueue.end(), _request->getBlockHashes().begin(), _request->getBlockHashes().end() );
}

template < class _Type >
void
CBitcoinNodeMedium< _Type >::add( CSetBloomFilterRequest< _Type > const * _request )
{
	LOCK( m_node->m_mediumLock );
	LastRequest = (common::CRequest< _Type >*)_request;
	m_node->m_filterSendQueue.push_back( _request->getBloomFilter() );

}

template < class _Type >
void
CBitcoinNodeMedium< _Type >::reloadResponses()
{
	m_responses.clear();

	m_responses.insert( std::make_pair( LastRequest, CRequestedMerkles( m_merkles, m_transactions,reinterpret_cast< long long >( m_node ) ) ) );
}

template < class _Type >
void
CBitcoinNodeMedium< _Type >::setResponse( CMerkleBlock const & _merkle )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_merkles.push_back( _merkle );
	reloadResponses();
}
// it is tricky  because for simplification tx and merkle have to go through the same channel in concurrent way
template < class _Type >
void
CBitcoinNodeMedium< _Type >::setResponse( CTransaction const & _tx )
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

#endif // BITCOIN_NODE_MEDIUM_H
