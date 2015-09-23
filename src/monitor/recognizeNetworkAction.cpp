// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/manageNetwork.h"
#include "common/actionHandler.h"
#include "common/setResponseVisitor.h"
#include "common/events.h"

#include "monitor/recognizeNetworkAction.h"
#include "monitor/connectNodeAction.h"
#include "monitor/filters.h"
#include "monitor/controller.h"
#include "monitor/provideInfoAction.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace monitor
{

int64_t const ConnectWaitTime = 10000;
struct CCheckAdmissionStatus;

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

		if ( _networkInfoEvent.m_valid )
		{
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
		}

		if (
			m_received.size() == m_alreadyAsked.size()
			 && nodesToAsk.empty()
			)
		{
			return transit< CCheckAdmissionStatus >();
		}
		else if ( !nodesToAsk.empty() )
		{
			context< CRecognizeNetworkAction >().forgetRequests();

			context< CRecognizeNetworkAction >().addRequest(
						new common::CTimeEventRequest(
							ConnectWaitTime
							, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

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
		return transit< CCheckAdmissionStatus >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CNetworkInfoResult >

	> reactions;

	std::set< common::CValidNodeInfo > m_trackers;
	std::set< common::CValidNodeInfo > m_monitors;
	std::set< CAddress > m_received;
	std::set< CAddress > m_alreadyAsked;
};

struct CCheckAdmissionStatus : boost::statechart::state< CCheckAdmissionStatus, CRecognizeNetworkAction >
{
	CCheckAdmissionStatus( my_context ctx ) : my_base( ctx )
	{
		context< CRecognizeNetworkAction >().forgetRequests();
		context< CRecognizeNetworkAction >().addRequest(
					new common::CScheduleActionRequest(
						  new CProvideInfoAction( common::CInfoKind::IsAddmited )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CRegistrationData const & _registerData )
	{
		CController::getInstance()->process_event( _registerData );
		context< CRecognizeNetworkAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CRegistrationData >
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
