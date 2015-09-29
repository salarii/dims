// Copyright (c) 2014-2015 DiMS dev-team
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
		std::vector< common::CNodeInfo > monitors, trackers;

		// problematic??  how to manage  self??

		std::vector< CAllyMonitorData > allyMonitors = CReputationTracker::getInstance()->getAllyMonitors();

		bool success;
		uintptr_t nodeIndicator;
		CAddress address;

		BOOST_FOREACH( CAllyMonitorData const & allyMonitorData, allyMonitors )
		{

			success = CReputationTracker::getInstance()->getKeyToNode( allyMonitorData.m_publicKey.GetID(), nodeIndicator );
			assert( success );

			success = CReputationTracker::getInstance()->getAddress( nodeIndicator, address );
			assert( success );
			//fix this
			monitors.push_back(
						common::CNodeInfo(
							allyMonitorData.m_key
							, address.ToStringIP()
							, address.GetPort()
							, common::CRole::Monitor ));
		}

		std::vector< CTrackerData > trackersData = CReputationTracker::getInstance()->getTrackers();

		BOOST_FOREACH( CTrackerData const & trackerData, trackersData )
		{

			success = CReputationTracker::getInstance()->getKeyToNode( trackerData.m_publicKey.GetID(), nodeIndicator );
			assert( success );

			success = CReputationTracker::getInstance()->getAddress( nodeIndicator, address );
			assert( success );

			trackers.push_back(
						common::CNodeInfo(
							trackerData.m_publicKey
							, address.ToStringIP()
							, address.GetPort()
							, common::CRole::Tracker ));
		}

		CMonitorData monitorData( trackers, monitors );

		CAuthenticationProvider::getInstance()->sign( hashMonitorData( monitorData ), monitorData.m_signed );

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

bool
CClientRequestsManager::getResponse( uint256 const & _token, ClientResponse & _clientResponse )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	InfoResponseRecord::iterator iterator = m_infoResponseRecord.find( _token );

	if ( iterator != m_infoResponseRecord.end() )
	{
		_clientResponse = iterator->second;
		m_infoResponseRecord.erase( iterator );

		return true;
	}

	return false;
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
