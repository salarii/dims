// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_CONTROLLER_STATES_H
#define TRACKER_CONTROLLER_STATES_H

#include <boost/statechart/transition.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/commonEvents.h"

#include "tracker/trackerController.h"
#include "tracker/trackerControllerEvents.h"

namespace tracker
{

struct CStandAlone;

struct CInitialSynchronization : boost::statechart::simple_state< CInitialSynchronization, CTrackerController >
{
	CInitialSynchronization();

	boost::statechart::result react( common::CSetScanBitcoinChainProgress const & _event );

	boost::statechart::result react( common::CUpdateStatus const & _event );

	boost::statechart::result react( common::CBitcoinNetworkConnection const & _event );

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CUpdateStatus >,
	boost::statechart::custom_reaction< common::CBitcoinNetworkConnection >,
	boost::statechart::custom_reaction< common::CSetScanBitcoinChainProgress >,
	boost::statechart::transition< common::CInitialSynchronizationDoneEvent, CStandAlone > > reactions;

	int m_blockLeft;
	unsigned int m_nodesNumber;
};


struct CConnected;
struct CLeading;
struct CSynchronizing;

struct CStandAlone : boost::statechart::state< CStandAlone, CTrackerController >
{
	CStandAlone( my_context  ctx );

	boost::statechart::result react( common::CNetworkRecognizedEvent const & _event );

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CNetworkRecognizedEvent > > reactions;
};

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
