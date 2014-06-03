// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "addTrackerAction.h"
#include "common/setResponseVisitor.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace monitor
{

struct CUninitiated : boost::statechart::simple_state< CUninitiated, CAddTrackerAction >
{
/*	typedef boost::mpl::list<
	boost::statechart::transition< CSwitchToConnectingEvent, CUnconnected >,
	boost::statechart::transition< CSwitchToConnectedEvent, CBothUnidentifiedConnected >
	> reactions;
*/
};
/*
struct CIdentified : boost::statechart::state< CIdentified, CAddTrackerAction >
{
	CIdentified( my_context ctx ) : my_base( ctx )
	{
		// for  now we  finish here
		context< CAddTrackerAction >().setRequest( 0 );
	}
};

*/
CAddTrackerAction::CAddTrackerAction( std::vector< unsigned char > const & _payload, unsigned int _mediumKind )
{
}


common::CRequest< MonitorResponses >*
CAddTrackerAction::execute()
{
	return 0;
}

void
CAddTrackerAction::accept( common::CSetResponseVisitor< MonitorResponses > & _visitor )
{

}

void
CAddTrackerAction::setRequest( common::CRequest< MonitorResponses >* _request )
{
	m_request = _request;
}

}
