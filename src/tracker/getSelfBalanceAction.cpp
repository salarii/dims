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

unsigned int const LoopTime = 10000;

struct CInitialState : boost::statechart::state< CInitialState, CGetBalanceFromNetworkAction >
{
	CInitialState( my_context ctx ) : my_base( ctx )
	{
		common::CInfoAskRequest< common::CTrackerTypes > * request =
				new common::CInfoAskRequest< common::CTrackerTypes >(
					common::CInfoKind::BalanceAsk
					, context< CGetBalanceFromNetworkAction >().getActionKey()
					, new CMediumClassFilter( common::CMediumKinds::Trackers, 1 ) );

//		request->setPayload( context< CGetBalanceFromNetworkAction >().getKeyId() );

		context< CGetBalanceFromNetworkAction >().addRequest( request );

		context< CGetBalanceFromNetworkAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CGetBalanceFromNetworkAction >().setExecuted();
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
struct CGetProveMessage : boost::statechart::state< CGetProveMessage, CGetBalanceFromNetworkAction >
{
	CGetProveMessage( my_context ctx ) : my_base( ctx )
	{}
};


struct CRetriveBalance : boost::statechart::state< CRetriveBalance, CGetBalanceFromNetworkAction >
{
	CRetriveBalance( my_context ctx ) : my_base( ctx )
	{}
};
// needed ??
struct CBalanceFromInternal : boost::statechart::state< CBalanceFromInternal, CGetBalanceFromNetworkAction >
{
	CBalanceFromInternal( my_context ctx ) : my_base( ctx )
	{}
};

CGetBalanceFromNetworkAction::CGetBalanceFromNetworkAction()
{
	initiate();
}

void
CGetBalanceFromNetworkAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
