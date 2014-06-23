// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_CONTROLLER_STATES_H
#define TRACKER_CONTROLLER_STATES_H

#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include "trackerController.h"
#include "trackerControllerEvents.h"

namespace tracker
{

struct CStandAlone : boost::statechart::state< CStandAlone, CTrackerController >
{
	CStandAlone( my_context  ctx );

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction< CGetStateEvent > > reactions;

	boost::statechart::result react( CGetStateEvent const & _event );
};


struct CConnected : boost::statechart::state< CConnected, CTrackerController >
{
	typedef boost::mpl::list<
	  boost::statechart::custom_reaction< CGetStateEvent > > reactions;

	CConnected( my_context ctx );

	boost::statechart::result react( CGetStateEvent const & _event )
	{
		return discard_event();
	}

};

}

#endif // TRACKER_CONTROLLER_STATES_H
