// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_CONTROLLER_STATES_H
#define TRACKER_CONTROLLER_STATES_H

#include <boost/statechart/transition.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include "trackerController.h"
#include "trackerControllerEvents.h"

namespace tracker
{

struct CStandAlone;

struct CInitialSynchronization : boost::statechart::simple_state< CInitialSynchronization, CTrackerController >
{
	CInitialSynchronization(){};

	typedef boost::statechart::transition< CInitialSynchronizationDoneEvent, CStandAlone > reactions;
};


struct CConnected;
struct CLeading;
struct CSynchronizing;

struct CStandAlone : boost::statechart::state< CStandAlone, CTrackerController >
{
	CStandAlone( my_context  ctx );

	typedef boost::mpl::list<
	boost::statechart::transition< CConnectedToTrackerEvent, CSynchronizing >,
	boost::statechart::transition< CTrackerConnectingEvent, CConnected > > reactions;
};
// not right, since  first connection will trigger synchronization( apply  wait  time to  allow  other connections to appear ??)
// this is outside action handler so I can't deffer this in "normal way"
// is  this irrelevant ???????, when monitors will came this  will change anyway ??????

struct CSynchronizing : boost::statechart::state< CSynchronizing, CTrackerController >
{
	CSynchronizing( my_context ctx );

	typedef boost::mpl::list<
	boost::statechart::transition< CSynchronizedWithNetworkEvent, CConnected > > reactions;
};

struct CConnected : boost::statechart::state< CConnected, CTrackerController >
{
	CConnected( my_context ctx );

	boost::statechart::result react( CGetStateEvent const & _event )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction< CGetStateEvent > > reactions;
};

}

#endif // TRACKER_CONTROLLER_STATES_H
