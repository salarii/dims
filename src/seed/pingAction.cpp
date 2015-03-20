// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "seed/pingAction.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace seed
{

struct CSendPing : boost::statechart::state< CSendPing, CPingAction >
{
	CSendPing( my_context ctx ) : my_base( ctx )
	{
		context< CPingAction >().dropRequests();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CSendPong >().dropRequests();

		context< CPingAction >().addRequests(
					new common::CPongRequest< common::CSeedTypes >(
						context< CPingAction >().getActionKey()
						, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CPingPongResult const & _pingPong )
	{
		assert( !_pingPong.m_isPing );// remove this  debug only

		m_received = true;

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CPingPongResult >
	> reactions;

	bool m_received;
};

struct CSendPong : boost::statechart::state< CSendPong, CPingAction >
{
	CSendPong( my_context ctx ) : my_base( ctx )
	{
		context< CSendPong >().dropRequests();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CSendPong >().dropRequests();

		if ( !m_received )
		{
			context< CPingAction >().addRequests(
						new common::CAcceptNodeAction< common::CSeedTypes >(
							context< CPingAction >().getActionKey()
							, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) ) );

		CSwitchToConnectedEvent
				CSwitchToConnectingEvent
		}
		else
		{
			context< CPingAction >().addRequests(
						new common::CPingRequest< common::CSeedTypes >(
							context< CPingAction >().getActionKey()
							, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) ) );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CPingPongResult const & _pingPong )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CPingPongResult >
	> reactions;

	bool m_received;
};

CPingAction::CPingAction( uint256 const & _actionKey, uintptr_t _nodeIndicator )
	: CCommunicationAction( _actionKey )
	, m_nodeIndicator( _nodeIndicator )
{
	initiate();
	process_event( common::CSwitchToConnectedEvent() );
}

void
CPingAction::accept( common::CSetResponseVisitor< common::CSeedTypes > & _visitor )
{
	_visitor.visit( *this );
}

uintptr_t
CPingAction::getNodeIndicator() const
{
	return m_nodeIndicator;
}

}
