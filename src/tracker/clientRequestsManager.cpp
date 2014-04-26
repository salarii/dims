// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clientRequestsManager.h"

#include <boost/foreach.hpp>

namespace tracker
{


class CHandleClientRequestVisitor : public boost::static_visitor< ClientResponse >
{
public:
	CHandleClientRequestVisitor(){};

	ClientResponse operator()( CTrackerStatsReq const & _transactionStatus ) const
	{

	}

	ClientResponse operator()( CAddressBalanceReq const & _addressBalanceReq ) const
	{

	}
};


uint256 CClientRequestsManager::ms_currentToken = 0;

CClientRequestsManager * CClientRequestsManager::ms_instance = NULL;

CClientRequestsManager::CClientRequestsManager()
{

}

CClientRequestsManager*
CClientRequestsManager::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CClientRequestsManager();
	};
	return ms_instance;
}

uint256
CClientRequestsManager::addRequest( NodeRequest const & _nodeRequest )
{
	// for transaction may use getHash in  the future??
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_getInfoRequest.insert( std::make_pair( ms_currentToken, _nodeRequest ) );
	return ms_currentToken++;
}

ClientResponse
CClientRequestsManager::getResponse( uint256 const & _token ) const
{
	// for transaction may use getHash in  the future??
	boost::lock_guard<boost::mutex> lock( m_lock );

	InfoResponseRecord::const_iterator iterator = m_infoResponseRecord.find( _token );
	if ( iterator != m_infoResponseRecord.end() )
		return iterator->second;
	else
		return CDummy();
}

void
CClientRequestsManager::processRequestLoop()
{
	while(1)
	{
		MilliSleep(1000);
		{
			boost::lock_guard<boost::mutex> lock( m_lock );

			BOOST_FOREACH( InfoRequestRecord::value_type request, m_getInfoRequest )
			{
				m_infoResponseRecord.insert( std::make_pair( request.first,boost::apply_visitor( CHandleClientRequestVisitor(), request.second ) ) );
			}

			m_getInfoRequest.clear();
		}
		
	}
}


}
