// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/requests.h"
#include "common/support.h"
#include "common/actionHandler.h"
#include "common/setResponseVisitor.h"
#include "common/events.h"

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
					new common::CTimeEventRequest( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CPingAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::Ping
					, common::CPing()
					, context< CPingAction >().getActionKey()
					, new CByKeyMediumFilter( context< CPingAction >().getPartnerKey() ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPingAction >().forgetRequests();

		if ( !m_received )
		{
		}
		else
		{
			context< CPingAction >().addRequest(
						new common::CTimeEventRequest( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

			context< CPingAction >().addRequest(
					new common::CSendMessageRequest(
						common::CPayloadKind::Ping
						, common::CPing()
						, context< CPingAction >().getActionKey()
						, new CByKeyMediumFilter( context< CPingAction >().getPartnerKey() ) ) );
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
					new common::CTimeEventRequest( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CPingAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::Pong
					, common::CPong()
					, context< CPingAction >().getActionKey()
					, new CByKeyMediumFilter( context< CPingAction >().getPartnerKey() ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPingAction >().forgetRequests();

		if ( !m_received )
		{
		}
		else
		{
			context< CPingAction >().addRequest(
						new common::CTimeEventRequest( PingPeriod, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

			context< CPingAction >().addRequest(
					new common::CSendMessageRequest(
						common::CPayloadKind::Pong
						, common::CPong()
						, context< CPingAction >().getActionKey()
						, new CByKeyMediumFilter( context< CPingAction >().getPartnerKey() ) ) );
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

CPingAction::CPingAction( CPubKey const & _partnerKey)
	: m_partnerKey( _partnerKey )
{
	initiate();
}

CPingAction::CPingAction( uint256 const & _actionKey, CPubKey const & _partnerKey )
	: m_partnerKey( _partnerKey )
{
	initiate();
}

void
CPingAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}

