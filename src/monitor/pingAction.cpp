// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/commonRequests.h"
#include "common/support.h"
#include "common/actionHandler.h"

#include "monitor/pingAction.h"
#include "monitor/filters.h"
#include "monitor/connectNodeAction.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

namespace monitor
{

struct CSendPing;
struct CSendPong;

int64_t PingPeriod = 20000;//milisec

std::set< uintptr_t >
CPingAction::m_pingedNodes;

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

		context< CPingAction >().addRequest(
					new common::CTimeEventRequest< common::CMonitorTypes >( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CPingAction >().addRequest(
					new common::CPingRequest< common::CMonitorTypes >(
						context< CPingAction >().getActionKey()
						, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPingAction >().dropRequests();

		if ( !m_received )
		{
			CConnectNodeAction * connectNode = new CConnectNodeAction(
						context< CPingAction >().getActionKey()
						, context< CPingAction >().getNodeIndicator() );

			connectNode->process_event( common::CSwitchToConnectingEvent() );

			common::CActionHandler< common::CMonitorTypes >::getInstance()->executeAction( connectNode );
		}
		else
		{
			context< CPingAction >().addRequest(
						new common::CTimeEventRequest< common::CMonitorTypes >( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

			context< CPingAction >().addRequest(
						new common::CPongRequest< common::CMonitorTypes >(
							context< CPingAction >().getActionKey()
							, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) ) );
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
		context< CPingAction >().dropRequests();

		context< CPingAction >().addRequest(
					new common::CTimeEventRequest< common::CMonitorTypes >( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CPingAction >().addRequest(
					new common::CPongRequest< common::CMonitorTypes >(
						context< CPingAction >().getActionKey()
						, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPingAction >().dropRequests();

		if ( !m_received )
		{
			CConnectNodeAction * connectNode = new CConnectNodeAction(
						context< CPingAction >().getActionKey()
						, context< CPingAction >().getNodeIndicator() );

			connectNode->process_event( common::CSwitchToConnectingEvent() );

			common::CActionHandler< common::CMonitorTypes >::getInstance()->executeAction( connectNode );
		}
		else
		{
			context< CPingAction >().addRequest(
						new common::CTimeEventRequest< common::CMonitorTypes >( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

			context< CPingAction >().addRequest(
						new common::CPongRequest< common::CMonitorTypes >(
							context< CPingAction >().getActionKey()
							, new CSpecificMediumFilter( context< CPingAction >().getNodeIndicator() ) ) );
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
CPingAction::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
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

