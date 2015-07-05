// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "tracker/passTransactionAction.h"

namespace tracker
{

struct CProcessAsClient;
struct CValidInNetwork;

struct CValidInNetworkEvent : boost::statechart::event< CValidInNetworkEvent >
{
};

struct CInvalidInNetworkEvent : boost::statechart::event< CInvalidInNetworkEvent >
{
};

struct CInitial : boost::statechart::simple_state< CInitial, CPassTransactionAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CValidInNetworkEvent, CValidInNetwork >,
	boost::statechart::transition< CInvalidInNetworkEvent, CProcessAsClient >
	> reactions;
};

struct CValidInNetwork : boost::statechart::state< CValidInNetwork, CGetSelfBalanceAction >
{
	CValidInNetwork( my_context ctx ) : my_base( ctx )
	{
	}
/*
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
	*/
};

CPassTransactionAction::CPassTransactionAction()
{
	initiate();
}

void
CPassTransactionAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
