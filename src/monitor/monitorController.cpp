// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "monitorController.h"

#include <boost/statechart/transition.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include <vector>

#include "net.h"

#include "common/actionHandler.h"
#include "common/manageNetwork.h"
#include "common/periodicActionExecutor.h"

#include "connectNodeAction.h"

#include "updateDataAction.h"

namespace monitor
{


struct CInitialSynchronizationDoneEvent : boost::statechart::event< CInitialSynchronizationDoneEvent >
{
};

struct CGetStateEvent : boost::statechart::event< CGetStateEvent >
{
};

struct CConnectedToTrackerEvent : boost::statechart::event< CConnectedToTrackerEvent >
{
	CConnectedToTrackerEvent(){}
};

struct CSynchronizedWithNetworkEvent : boost::statechart::event< CSynchronizedWithNetworkEvent >
{
};

struct CTrackerConnectingEvent : boost::statechart::event< CTrackerConnectingEvent >
{
	CTrackerConnectingEvent(){}
};

struct CConnectWithTrackerRequest : boost::statechart::event< CConnectWithTrackerRequest >
{
	CConnectWithTrackerRequest( std::string const & _trackerAddress ):m_trackerAddress( _trackerAddress ){};

	std::string const m_trackerAddress;
};

struct CMonitorStandAlone;

struct CMonitorInitialSynchronization : boost::statechart::simple_state< CMonitorInitialSynchronization, CMonitorController >
{
	CMonitorInitialSynchronization(){};

	typedef boost::statechart::transition< CInitialSynchronizationDoneEvent, CMonitorStandAlone > reactions;
};


struct CMonitorConnected;
struct CMonitorSynchronizing;

struct CMonitorStandAlone : boost::statechart::state< CMonitorStandAlone, CMonitorController >
{
	CMonitorStandAlone( my_context ctx ) : my_base( ctx )
	{
		// search for  seeder  action
		std::vector<CAddress> vAdd;

		common::CManageNetwork::getInstance()->getIpsFromSeed( vAdd );

		if ( !vAdd.empty() )
		{
			BOOST_FOREACH( CAddress address, vAdd )
			{
				common::CActionHandler< common::CMonitorTypes >::getInstance()->executeAction( new CConnectNodeAction( address ) );
			}
		}
		else
		{
			common::CManageNetwork::getInstance()->getSeedIps( vAdd );

			// let know seed about our existence
			BOOST_FOREACH( CAddress address, vAdd )
			{
				common::CActionHandler< common::CMonitorTypes >::getInstance()->executeAction( new CConnectNodeAction( address ) );
			}
		}

		/// move it to different place

//		common::CPeriodicActionExecutor< common::CMonitorTypes >::getInstance()->addAction( new CUpdateDataAction( false ), 10000 );
	}

	typedef boost::mpl::list<
	boost::statechart::transition< CConnectedToTrackerEvent, CMonitorSynchronizing >,
	boost::statechart::transition< CTrackerConnectingEvent, CMonitorConnected > > reactions;
};
// not right, since  first connection will trigger synchronization( apply  wait  time to  allow  other connections to appear ??)
// this is outside action handler so I can't deffer this in "normal way"
// is  this irrelevant ???????, when monitors will came this  will change anyway ??????

struct CMonitorSynchronizing : boost::statechart::state< CMonitorSynchronizing, CMonitorController >
{

	CMonitorSynchronizing( my_context ctx ) : my_base( ctx )
	{
	/*	CSynchronizationAction * synchronizationAction = new CSynchronizationAction();

		common::CActionHandler< common::CMonitorTypes >::getInstance()->executeAction( synchronizationAction );
		synchronizationAction->process_event( CSwitchToSynchronizing() );
*/
	}

	typedef boost::mpl::list<
	boost::statechart::transition< CSynchronizedWithNetworkEvent, CMonitorConnected > > reactions;
};

struct CMonitorConnected : boost::statechart::state< CMonitorConnected, CMonitorController >
{
	CMonitorConnected( my_context ctx ) : my_base( ctx )
	{
	//CMonitorController::getInstance()->setConnected( true );
	}

	boost::statechart::result react( CGetStateEvent const & _event )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction< CGetStateEvent > > reactions;
};

CMonitorController * CMonitorController::ms_instance = NULL;

CMonitorController*
CMonitorController::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CMonitorController();
	};
	return ms_instance;
}

CMonitorController::CMonitorController()
	: m_price( 0 )
	, m_period( -1 )
{
	initiate();
}

}
