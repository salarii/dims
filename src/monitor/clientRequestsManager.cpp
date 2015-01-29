// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clientRequestsManager.h"

#include "common/actionHandler.h"
#include "common/authenticationProvider.h"
#include "monitor/reputationTracer.h"

#include "base58.h"

#include <boost/foreach.hpp>

using namespace common;

namespace monitor
{

class CHandleClientRequestVisitor : public common::CClientRequestVisitorHandlerBase
{
public:
	using CClientRequestVisitorHandlerBase::operator ();

	CHandleClientRequestVisitor(uint256 const & _hash):m_hash( _hash ){};

	void operator()( CNetworkInfoReq const & _networkInfoReq ) const
	{
		// handle  it through  action  handler??
		std::vector< common::CValidNodeInfo > validNodesInfo;

		//BOOST_FOREACH( common::CValidNodeInfo const & validNodeInfo, CTrackerNodesManager::getInstance()->getValidNodes() )
		{
		//	validNodesInfo.push_back( validNodeInfo );
		}
		CClientRequestsManager::getInstance()->setClientResponse( m_hash, CClientNetworkInfoResult( validNodesInfo, common::CAuthenticationProvider::getInstance()->getMyKey(), common::CRole::Monitor ) );
	}

	void operator()( CMonitorInfoReq const & _MonitorInfoReq ) const
	{
		std::vector< CPubKey > monitors;
		monitors.push_back( common::CAuthenticationProvider::getInstance()->getMyKey() );

		std::vector< CPubKey > allyMonitors =  CReputationTracker::getInstance()->getAllyMonitors();
		// risky in  this  way
		monitors.insert( monitors.end(), allyMonitors.begin(), allyMonitors.end() );

		CMonitorData monitorData( CReputationTracker::getInstance()->getTrackers(), monitors );

		CClientRequestsManager::getInstance()->setClientResponse( m_hash, monitorData );
	}
private:
	uint256 const m_hash;
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
CClientRequestsManager::addRequest( NodeRequests const & _nodeRequest )
{
	boost::lock_guard<boost::mutex> lock( m_requestLock );
	m_getInfoRequest.insert( std::make_pair( ms_currentToken, _nodeRequest ) );
	return ms_currentToken++;
}

void
CClientRequestsManager::addRequest( NodeRequests const & _nodeRequest, uint256 const & _hash )
{
	boost::lock_guard<boost::mutex> lock( m_requestLock );
	m_getInfoRequest.insert( std::make_pair( _hash, _nodeRequest ) );
}

ClientResponse
CClientRequestsManager::getResponse( uint256 const & _token )
{
	// for transaction may use getHash in  the future??
	boost::lock_guard<boost::mutex> lock( m_lock );

	InfoResponseRecord::iterator iterator = m_infoResponseRecord.find( _token );

	ClientResponse response;
	if ( iterator != m_infoResponseRecord.end() )
	{
		response = iterator->second;
		m_infoResponseRecord.erase( iterator );
	}
	else
	{
		CDummy dummy;
		dummy.m_token = _token;
		response = dummy;
	}

	return response;
}

void
CClientRequestsManager::setClientResponse( uint256 const & _hash, ClientResponse const & _clientResponse )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	if ( m_infoResponseRecord.find( _hash ) != m_infoResponseRecord.end() )
		m_infoResponseRecord.erase(_hash);
	m_infoResponseRecord.insert( std::make_pair( _hash, _clientResponse ) );
}

void
CClientRequestsManager::processRequestLoop()
{
	while(1)
	{
		MilliSleep(1000);
		{
			boost::lock_guard<boost::mutex> lock( m_requestLock );

			BOOST_FOREACH( InfoRequestRecord::value_type request, m_getInfoRequest )
			{
				boost::apply_visitor( CHandleClientRequestVisitor( request.first ), request.second );
			}

			m_getInfoRequest.clear();
		}
		
	}
}


}
