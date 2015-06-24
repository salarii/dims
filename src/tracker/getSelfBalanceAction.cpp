// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/setResponseVisitor.h"
#include "common/commonRequests.h"

#include "tracker/getSelfBalanceAction.h"
#include "tracker/trackerFilters.h"

namespace tracker
{

struct CInitialState : boost::statechart::state< CInitialState, CGetSelfBalanceAction >
{
	CInitialState( my_context ctx ) : my_base( ctx )
	{
// check  status ???
		context< CGetSelfBalanceAction >().addRequest(
					new common::CBalanceRequest< common::CTrackerTypes >(
						new CMediumClassFilter( common::CMediumKinds::Trackers, 1 ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

// will be neeeded at some point ??
struct CGetProveMessage : boost::statechart::state< CGetProveMessage, CGetSelfBalanceAction >
{
	CGetProveMessage( my_context ctx ) : my_base( ctx )
	{}
};


struct CRetriveBalance : boost::statechart::state< CRetriveBalance, CGetSelfBalanceAction >
{
	CRetriveBalance( my_context ctx ) : my_base( ctx )
	{}
};
// needed ??
struct CBalanceFromInternal : boost::statechart::state< CBalanceFromInternal, CGetSelfBalanceAction >
{
	CBalanceFromInternal( my_context ctx ) : my_base( ctx )
	{}
};

CGetSelfBalanceAction::CGetSelfBalanceAction()
{
	initiate();
}

void
CGetSelfBalanceAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
