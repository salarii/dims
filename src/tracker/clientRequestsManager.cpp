// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clientRequestsManager.h"
#include "common/actionHandler.h"
#include "getBalanceAction.h"
#include "base58.h"

#include <boost/foreach.hpp>

using namespace common;

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
		CKeyID keyId;
		CBitcoinAddress( _addressBalanceReq.m_address ).GetKeyID( keyId );
		common::CActionHandler< TrackerResponses >::getInstance()->executeAction( (common::CAction< TrackerResponses >*)new CGetBalanceAction( keyId ) );
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
CClientRequestsManager::getResponse( uint256 const & _token )
{
	// for transaction may use getHash in  the future??
	boost::lock_guard<boost::mutex> lock( m_lock );

	InfoResponseRecord::iterator iterator = m_infoResponseRecord.find( _token );

	common::ClientResponse response;
	if ( iterator != m_infoResponseRecord.end() )
	{
		response = iterator->second;
	}
	else
	{
		CDummy dummy;
		dummy.m_token = _token;
		response = dummy;
	}

	m_infoResponseRecord.erase( iterator );

	return response;
}

void
CClientRequestsManager::setClientResponse( uint256 const & _hash, common::ClientResponse const & _clientResponse )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	m_infoResponseRecord.insert( std::make_pair( _hash, _clientResponse ) );
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
