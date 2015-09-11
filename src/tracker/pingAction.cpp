// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/commonRequests.h"
#include "common/support.h"
#include "common/actionHandler.h"

#include "tracker/pingAction.h"
#include "tracker/filters.h"
#include "tracker/connectNodeAction.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace tracker
{

struct CSendPing;
struct CSendPong;

std::set< uintptr_t >
CPingAction::m_pingedNodes;

int64_t PingPeriod = 20000;//milisec

struct CUninitialised : boost::statechart::state< CUninitialised, CPingAction >
{
	CUninitialised( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("ping action: %p uninitialised \n", &context< CPingAction >() );
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
		LogPrintf("ping action: %p send ping \n", &context< CPingAction >() );
		context< CPingAction >().forgetRequests();

		context< CPingAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		common::CSendMessageRequest< common::CTrackerTypes > * request =
				new common::CSendMessageRequest< common::CTrackerTypes >(
					common::CPayloadKind::Ping
					, context< CPingAction >().getActionKey()
					, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) );

		request->addPayload( common::CPing() );

		context< CPingAction >().addRequest( request );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPingAction >().forgetRequests();

		if ( !m_received )
		{
			CConnectNodeAction * connectNode = new CConnectNodeAction(
						context< CPingAction >().getActionKey()
						, context< CPingAction >().getNodeIndicator() );

			connectNode->process_event( common::CSwitchToConnectingEvent() );

			common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( connectNode );
		}
		else
		{
			context< CPingAction >().addRequest(
						new common::CTimeEventRequest< common::CTrackerTypes >( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

			common::CSendMessageRequest< common::CTrackerTypes > * request =
					new common::CSendMessageRequest< common::CTrackerTypes >(
						common::CPayloadKind::Ping
						, context< CPingAction >().getActionKey()
						, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) );

			request->addPayload( common::CPing() );

			context< CPingAction >().addRequest( request );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CPingPongResult const & _pingPong )
	{
		assert( !_pingPong.m_isPing );// remove this  debug only

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
		LogPrintf("ping action: %p send pong \n", &context< CPingAction >() );
		context< CPingAction >().forgetRequests();

		context< CPingAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		common::CSendMessageRequest< common::CTrackerTypes > * request =
				new common::CSendMessageRequest< common::CTrackerTypes >(
					common::CPayloadKind::Pong
					, context< CPingAction >().getActionKey()
					, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) );

		request->addPayload( common::CPong() );

		context< CPingAction >().addRequest( request );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPingAction >().forgetRequests();

		if ( !m_received )
		{
			CConnectNodeAction * connectNode = new CConnectNodeAction(
						context< CPingAction >().getActionKey()
						, context< CPingAction >().getNodeIndicator() );

			connectNode->process_event( common::CSwitchToConnectingEvent() );

			common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( connectNode );
		}
		else
		{
			context< CPingAction >().addRequest(
						new common::CTimeEventRequest< common::CTrackerTypes >( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

			common::CSendMessageRequest< common::CTrackerTypes > * request =
					new common::CSendMessageRequest< common::CTrackerTypes >(
						common::CPayloadKind::Pong
						, context< CPingAction >().getActionKey()
						, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) );

			request->addPayload( common::CPong() );

			context< CPingAction >().addRequest( request );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CPingPongResult const & _pingPong )
	{
		assert( _pingPong.m_isPing );// remove this  debug only

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
	m_pingedNodes.insert( _nodeIndicator );
	initiate();
}

CPingAction::CPingAction( uint256 const & _actionKey, uintptr_t _nodeIndicator )
	: m_nodeIndicator( _nodeIndicator )
{
	m_pingedNodes.insert( _nodeIndicator );
	initiate();
}

void
CPingAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

uintptr_t
CPingAction::getNodeIndicator() const
{
	return m_nodeIndicator;
}

bool
CPingAction::isPinged( uintptr_t _nodeIndicator )
{
	return m_pingedNodes.find( _nodeIndicator ) != m_pingedNodes.end();
}

}

