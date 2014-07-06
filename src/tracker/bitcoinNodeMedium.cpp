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
	if ( m_responses.empty() )
		_requestResponse.push_back( common::CContinueResult( 0 ) );
	else
		_requestResponse = m_responses;


	return true;
}

void
CBitcoinNodeMedium::clearResponses()
{
	m_responses.clear();
}


void
CBitcoinNodeMedium::add( CAskForTransactionsRequest const * _request )
{
	boost::lock_guard<boost::mutex> lock( m_node->m_mediumLock );
	m_node->m_blockQueue.insert( m_node->m_blockQueue.end(), _request->getBlockHashes().begin(), _request->getBlockHashes().end() );
}

void
CBitcoinNodeMedium::add( CSetBloomFilterRequest const * _request )
{
	boost::lock_guard<boost::mutex> lock( m_node->m_mediumLock );
	m_node->m_filterSendQueue.push_back( _request->getBloomFilter() );
}

void
CBitcoinNodeMedium::setResponse( TrackerResponses const & _response )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_responses.push_back( _response );
}

}
