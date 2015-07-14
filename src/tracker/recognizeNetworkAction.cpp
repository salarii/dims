// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/manageNetwork.h"
#include "common/actionHandler.h"

#include "tracker/recognizeNetworkAction.h"
#include "tracker/connectNodeAction.h"
#include "tracker/trackerFilters.h"
#include "tracker/trackerController.h"
#include "tracker/provideInfoAction.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace tracker
{

struct CGetDnsInfo : boost::statechart::state< CGetDnsInfo, CRecognizeNetworkAction >
{
	CGetDnsInfo( my_context ctx ) : my_base( ctx )
	{
		// search for  seeder  action
		std::vector<CAddress> vAdd;

		common::CManageNetwork::getInstance()->getIpsFromSeed( vAdd );

		if ( !vAdd.empty() )
		{
			BOOST_FOREACH( CAddress address, vAdd )
			{
				context< CRecognizeNetworkAction >().dropRequests();
				context< CRecognizeNetworkAction >().addRequest(
							new common::CScheduleActionRequest< common::CTrackerTypes >(
								new CConnectNodeAction( address )
								, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
			}
		}
		else
		{
			common::CManageNetwork::getInstance()->getSeedIps( vAdd );

			if ( !vAdd.empty() )
			{
				context< CRecognizeNetworkAction >().setExit();
				return;
			}
			// let know seed about our existence
			BOOST_FOREACH( CAddress address, vAdd )
			{
				context< CRecognizeNetworkAction >().dropRequests();
				context< CRecognizeNetworkAction >().addRequest(
							new common::CScheduleActionRequest< common::CTrackerTypes >(
								new CConnectNodeAction( address )
								, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
			}
		}
	}

	boost::statechart::result react( common::CNetworkInfoEvent const & _networkInfoEvent )
	{
		std::set< common::CValidNodeInfo > nodesToAsk;

		if ( _networkInfoEvent.m_role == common::CRole::Tracker )
		{
			m_trackers.insert( _networkInfoEvent.m_self );
		}
		else if ( _networkInfoEvent.m_role == common::CRole::Monitor )
		{
			m_monitors.insert( _networkInfoEvent.m_self );
		}

		BOOST_FOREACH( common::CValidNodeInfo const & nodeInfo, _networkInfoEvent.m_monitorsInfo )
		{
			if ( m_monitors.find( nodeInfo ) == m_monitors.end() )
			{
				nodesToAsk.insert( nodeInfo );
			}
		}

		BOOST_FOREACH( common::CValidNodeInfo const & nodeInfo, _networkInfoEvent.m_trackersInfo )
		{
			if ( m_trackers.find( nodeInfo ) == m_trackers.end() )
			{
				nodesToAsk.insert( nodeInfo );
			}
		}

		if ( nodesToAsk.empty() )
		{
			context< CRecognizeNetworkAction >().setExit();
		}
		else
		{
			BOOST_FOREACH( common::CValidNodeInfo const & nodeInfo, nodesToAsk )
			{
				context< CRecognizeNetworkAction >().addRequest(
							new common::CScheduleActionRequest< common::CTrackerTypes >(
								new CConnectNodeAction( nodeInfo.m_address )
								, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
			}
		}
		return discard_event();
	}

	~CGetDnsInfo()
	{
		CTrackerController::getInstance()->process_event( common::CNetworkRecognizedEvent( m_trackers, m_monitors ) );
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CNetworkInfoEvent >
	> reactions;

	std::set< common::CValidNodeInfo > m_trackers;
	std::set< common::CValidNodeInfo > m_monitors;

};

struct CCheckRegistrationStatus : boost::statechart::state< CCheckRegistrationStatus, CRecognizeNetworkAction >
{
	CCheckRegistrationStatus( my_context ctx ) : my_base( ctx )
	{
		context< CRecognizeNetworkAction >().dropRequests();
		context< CRecognizeNetworkAction >().addRequest(
					new common::CScheduleActionRequest< common::CTrackerTypes >(
						  new CProvideInfoAction( common::CInfoKind::IsRegistered )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CRegistrationDataEvent const & _registerData )
	{

	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CRegistrationDataEvent >
	> reactions;
};


CRecognizeNetworkAction::CRecognizeNetworkAction()
{
	initiate();
}

void
CRecognizeNetworkAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
