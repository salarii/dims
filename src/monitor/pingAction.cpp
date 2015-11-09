// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/requests.h"
#include "common/support.h"
#include "common/actionHandler.h"
#include "common/setResponseVisitor.h"
#include "common/events.h"
#include "common/selfNode.h"

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

namespace
{
	common::CSelfNode * SelfNode;
}

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
	CSendPing( my_context ctx ) : my_base( ctx )
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
					, new CSpecificMediumFilter( common::convertToInt( SelfNode ) ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPingAction >().forgetRequests();

		SelfNode->fDisconnect = true;
		context< CPingAction >().setExit();

		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
			common::CMessage orginalMessage;
			if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey) )
				assert( !"service it somehow" );

			if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::Pong )
			{
				context< CPingAction >().setExit();
			}
			return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

struct CSendPong : boost::statechart::state< CSendPong, CPingAction >
{
	CSendPong( my_context ctx ) : my_base( ctx )
	{
		LogPrintf("ping action: %p send pong \n", &context< CPingAction >() );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::Ping)
		{
			common::CResult result;

			common::convertPayload( orginalMessage, result );

			context< CPingAction >().addRequest(
						new common::CSendMessageRequest(
							common::CPayloadKind::Pong
							, common::CPong()
							, context< CPingAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

			context< CPingAction >().setExit();
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

};

CPingAction::CPingAction( common::CSelfNode * _node )
{
	SelfNode = _node;
	initiate();
	process_event( common::CStartPingEvent() );
}

CPingAction::CPingAction( uint256 const & _actionKey)
	: common::CAction( _actionKey )
{
	initiate();
	process_event( common::CStartPongEvent() );

}

void
CPingAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}

