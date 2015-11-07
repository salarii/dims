// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/manageNetwork.h"
#include "common/actionHandler.h"
#include "common/setResponseVisitor.h"

#include "tracker/recognizeNetworkAction.h"
#include "tracker/connectNodeAction.h"
#include "tracker/filters.h"
#include "tracker/controller.h"
#include "tracker/provideInfoAction.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace tracker
{

struct CCheckRegistrationStatus;

int64_t const ConnectWaitTime = 60000;

struct CGetDnsInfo : boost::statechart::state< CGetDnsInfo, CRecognizeNetworkAction >
{
	CGetDnsInfo( my_context ctx ) : my_base( ctx )
	{
		// search for  seeder  action
		std::vector<CAddress> vAdd;

		common::CManageNetwork::getInstance()->getIpsFromSeed( vAdd );

		context< CRecognizeNetworkAction >().forgetRequests();
		context< CRecognizeNetworkAction >().addRequest(
					new common::CTimeEventRequest(
						  ConnectWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		if ( !vAdd.empty() )
		{
			BOOST_FOREACH( CAddress address, vAdd )
			{
				m_alreadyAsked.insert( address );
				context< CRecognizeNetworkAction >().addRequest(
							new common::CScheduleActionRequest(
								new CConnectNodeAction( address )
								, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
			}
		}
		else
		{
			common::CManageNetwork::getInstance()->getSeedIps( vAdd );

			if ( vAdd.empty() )
			{
				context< CRecognizeNetworkAction >().setExit();
			}

			// let know seed about our existence
			BOOST_FOREACH( CAddress address, vAdd )
			{
				m_alreadyAsked.insert( address );
				context< CRecognizeNetworkAction >().addRequest(
							new common::CScheduleActionRequest(
								new CConnectNodeAction( address )
								, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
			}
		}
	}

	boost::statechart::result react( common::CNetworkInfoResult const & _networkInfoEvent )
	{
		std::set< CAddress > nodesToAsk;

		m_received.insert( _networkInfoEvent.m_nodeSelfInfo.m_address );//  valid  even  if  invalid : )

		if ( _networkInfoEvent.m_role == common::CRole::Tracker )
		{
			m_trackers.insert( _networkInfoEvent.m_nodeSelfInfo );
		}
		else if ( _networkInfoEvent.m_role == common::CRole::Monitor )
		{
			m_monitors.insert( _networkInfoEvent.m_nodeSelfInfo );
		}

		BOOST_FOREACH( common::CValidNodeInfo const & nodeInfo, _networkInfoEvent.m_monitorsInfo )
		{
			if ( m_monitors.find( nodeInfo ) == m_monitors.end() )
			{
				if ( m_alreadyAsked.find( nodeInfo.m_address ) == m_alreadyAsked.end() )
					nodesToAsk.insert( nodeInfo.m_address );
			}
		}

		BOOST_FOREACH( common::CValidNodeInfo const & nodeInfo, _networkInfoEvent.m_trackersInfo )
		{
			if ( m_trackers.find( nodeInfo ) == m_trackers.end() )
			{
				if ( m_alreadyAsked.find( nodeInfo.m_address ) == m_alreadyAsked.end() )
					nodesToAsk.insert( nodeInfo.m_address );
			}
		}

		if (
				m_received.size() == m_alreadyAsked.size()
				&& nodesToAsk.empty()
				)
		{
			context< CRecognizeNetworkAction >().forgetRequests();
			return transit< CCheckRegistrationStatus >();
		}
		else if ( !nodesToAsk.empty() )
		{
			context< CRecognizeNetworkAction >().forgetRequests();

			BOOST_FOREACH( CAddress const & address, nodesToAsk )
			{
				m_alreadyAsked.insert( address );
				context< CRecognizeNetworkAction >().addRequest(
							new common::CScheduleActionRequest(
								new CConnectNodeAction( address )
								, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
			}
		}

		return discard_event();
	}

	~CGetDnsInfo()
	{
		CController::getInstance()->process_event( common::CNetworkRecognizedData( m_trackers, m_monitors ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CRecognizeNetworkAction >().forgetRequests();
		return transit< CCheckRegistrationStatus >();
	}

	boost::statechart::result react( common::CFailureEvent const & _failureEvent )
	{
		CAddress problemNode;

			common::readPayload( _failureEvent.m_problemData, problemNode );
			m_received.insert( problemNode );

			if (m_received.size() == m_alreadyAsked.size() )
			{
				context< CRecognizeNetworkAction >().forgetRequests();
				return transit< CCheckRegistrationStatus >();
			}

			return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CNetworkInfoResult >,
	boost::statechart::custom_reaction< common::CFailureEvent >
	> reactions;

	std::set< common::CValidNodeInfo > m_trackers;
	std::set< common::CValidNodeInfo > m_monitors;
	std::set< CAddress > m_received;
	std::set< CAddress > m_alreadyAsked;
};

struct CCheckRegistrationStatus : boost::statechart::state< CCheckRegistrationStatus, CRecognizeNetworkAction >
{
	CCheckRegistrationStatus( my_context ctx ) : my_base( ctx )
	{
		context< CRecognizeNetworkAction >().forgetRequests();
		context< CRecognizeNetworkAction >().addRequest(
					new common::CScheduleActionRequest(
						  new CProvideInfoAction( common::CInfoKind::IsRegistered, common::CMediumKinds::Monitors )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CRegistrationData const & _registerData )
	{
		CController::getInstance()->process_event( _registerData );
		context< CRecognizeNetworkAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CFailureEvent const & _failureEvent )
	{
		context< CRecognizeNetworkAction >().setExit();

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CRegistrationData >,
	boost::statechart::custom_reaction< common::CFailureEvent >
	> reactions;

};


CRecognizeNetworkAction::CRecognizeNetworkAction()
{
	initiate();
}

void
CRecognizeNetworkAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}
