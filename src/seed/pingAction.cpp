// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/mediumRequests.h"
#include "common/support.h"

#include "seed/pingAction.h"
#include "seed/seedFilter.h"
#include "seed/acceptNodeAction.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace seed
{

struct CSendPing;
struct CSendPong;

int64_t PingPeriod = 20;

struct CUninitialised : boost::statechart::state< CUninitialised, CPingAction >
{
	CUninitialised( my_context ctx ) : my_base( ctx )
	{
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CStartPingEvent, CSendPing >,
	boost::statechart::transition< common::CStartPongEvent, CSendPong >
	> reactions;
};

struct CSendPing : boost::statechart::state< CSendPing, CPingAction >
{
	CSendPing( my_context ctx ) : my_base( ctx ), m_received( true )
	{
		context< CPingAction >().dropRequests();

		context< CPingAction >().addRequests(
					new common::CTimeEventRequest< common::CSeedTypes >( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CPingAction >().addRequests(
					new common::CPingRequest< common::CSeedTypes >(
						context< CPingAction >().getActionKey()
						, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPingAction >().dropRequests();

		if ( !m_received )
		{
			context< CPingAction >().cleanup();

			CAcceptNodeAction * acceptAction = new CAcceptNodeAction(
						context< CPingAction >().getActionKey()
						, context< CPingAction >().getNodeIndicator() );

			acceptAction->process_event( common::CSwitchToConnectingEvent() );

			common::CActionHandler< common::CSeedTypes >::getInstance()->executeAction( acceptAction );
		}
		else
		{
			context< CPingAction >().addRequests(
						new common::CTimeEventRequest< common::CSeedTypes >( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

			context< CPingAction >().addRequests(
						new common::CPingRequest< common::CSeedTypes >(
							context< CPingAction >().getActionKey()
							, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) ) );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CPingPongResult const & _pingPong )
	{
		assert( _pingPong.m_isPing );// remove this  debug only

		if ( !_pingPong.m_isPing )
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
		context< CPingAction >().dropRequests();

		context< CPingAction >().addRequests(
					new common::CTimeEventRequest< common::CSeedTypes >( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CPingAction >().addRequests(
					new common::CPongRequest< common::CSeedTypes >(
						context< CPingAction >().getActionKey()
						, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPingAction >().dropRequests();

		if ( !m_received )
		{
			context< CPingAction >().cleanup();

			CAcceptNodeAction * acceptAction = new CAcceptNodeAction(
						context< CPingAction >().getActionKey()
						, context< CPingAction >().getNodeIndicator() );

			acceptAction->process_event( common::CSwitchToConnectingEvent() );

			common::CActionHandler< common::CSeedTypes >::getInstance()->executeAction( acceptAction );
		}
		else
		{
			context< CPingAction >().addRequests(
						new common::CTimeEventRequest< common::CSeedTypes >( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

			context< CPingAction >().addRequests(
						new common::CPongRequest< common::CSeedTypes >(
							context< CPingAction >().getActionKey()
							, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) ) );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CPingPongResult const & _pingPong )
	{
		assert( !_pingPong.m_isPing );// remove this  debug only

		if ( _pingPong.m_isPing )
			m_received = true;

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CPingPongResult >
	> reactions;

	bool m_received;
};

CPingAction::CPingAction( uintptr_t _nodeIndicator )
	: m_nodeIndicator( _nodeIndicator )
{
	initiate();
}

CPingAction::CPingAction( uint256 const & _actionKey, uintptr_t _nodeIndicator )
	: CCommunicationAction( _actionKey )
	, m_nodeIndicator( _nodeIndicator )
{
	initiate();
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

void
CPingAction::cleanup() const
{
	CSeedNodesManager::getInstance()->clearPublicKey( m_nodeIndicator );
	common::CSeedBaseMedium * medium =
			CSeedNodesManager::getInstance()->eraseMedium( m_nodeIndicator );

	if ( medium )
		delete medium;
}

}
