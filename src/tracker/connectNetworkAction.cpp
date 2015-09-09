// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/actionHandler.h"

#include "tracker/connectNetworkAction.h"
#include "tracker/filters.h"
#include "tracker/controller.h"
#include "tracker/synchronizationAction.h"
#include "tracker/controllerEvents.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace tracker
{

struct CInitialConnect : public boost::statechart::state< CInitialConnect, CConnectNetworkAction >
{
	CInitialConnect( my_context ctx )
		: my_base( ctx )
	{
		// check  sanity
		common::CRegistrationData registrationData = CController::getInstance()->getRegistrationData();

		int64_t time = GetTime() - registrationData.m_registrationTime;
		if ( registrationData.m_period < time )
		{
			context< CConnectNetworkAction >().setExit();
			return;
		}

		context< CConnectNetworkAction >().forgetRequests();

		uintptr_t nodeIndicator;
		if ( !CTrackerNodesManager::getInstance()->getKeyToNode( registrationData.m_key, nodeIndicator))
		{
			context< CConnectNetworkAction >().setExit();
			return;
		}

		m_key = registrationData.m_key;
		context< CConnectNetworkAction >().addRequest(
					new common::CScheduleActionRequest< common::CTrackerTypes >(
						new CSynchronizationAction( nodeIndicator )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CSynchronizationResult const & _synchronizationResult )
	{
		if ( _synchronizationResult.m_result )
		{
			CController::getInstance()->process_event( CMonitorAcceptEvent( m_key ) );
		}

		context< CConnectNetworkAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CSynchronizationResult >
	> reactions;

	CPubKey m_key;
};

CConnectNetworkAction::CConnectNetworkAction()
{
	initiate();
}

void
CConnectNetworkAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
