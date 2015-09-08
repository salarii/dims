// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectNodeAction.h"
#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"
#include "common/authenticationProvider.h"
#include "common/commonRequests.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "tracker/provideInfoAction.h"
#include "tracker/requests.h"

namespace tracker
{

unsigned int const LoopTime = 20000;//milisec

struct CProvideInfo;
struct CAskForInfo;

struct CAskForInfoEvent : boost::statechart::event< CAskForInfoEvent >
{
};

struct CProvideInfoEvent : boost::statechart::event< CProvideInfoEvent >
{
};

struct CInit : boost::statechart::state< CInit, CProvideInfoAction >
{
	CInit( my_context ctx ) : my_base( ctx )
	{}

	typedef boost::mpl::list<
	boost::statechart::transition< CProvideInfoEvent, CProvideInfo >,
	boost::statechart::transition< CAskForInfoEvent, CAskForInfo >
	> reactions;

};

struct CProvideInfo : boost::statechart::state< CProvideInfo, CProvideInfoAction >
{
	CProvideInfo( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		context< CProvideInfoAction >().dropRequests();
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CInfoRequestData requestedInfo;

		common::convertPayload( orginalMessage, requestedInfo );

		context< CProvideInfoAction >().dropRequests();
		context< CProvideInfoAction >().addRequest( new CDeliverInfoRequest( context< CProvideInfoAction >().getActionKey(), new CSpecificMediumFilter( context< CProvideInfoAction >().getNodeIndicator() ) ) );

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CAskForInfo : boost::statechart::state< CAskForInfo, CProvideInfoAction >
{
	CAskForInfo( my_context ctx ) : my_base( ctx )
	{
		context< CProvideInfoAction >().addRequest( new common::CInfoAskRequest< common::CTrackerTypes >(
														  context< CProvideInfoAction >().getInfo()
														, context< CProvideInfoAction >().getActionKey()
														, new CMediumClassFilter( common::CMediumKinds::Monitors, 1 ) ) );

		context< CProvideInfoAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						  LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CProvideInfoAction >().dropRequests();
		context< CProvideInfoAction >().setExit();

		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{

		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		context< CProvideInfoAction >().addRequest(
					new common::CAckRequest< common::CTrackerTypes >(
						  context< CProvideInfoAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

		if ( ( common::CPayloadKind::Enum )orginalMessage.m_header.m_payloadKind == common::CPayloadKind::ValidRegistration )
		{
			common::CValidRegistration validRegistration;

			common::convertPayload( orginalMessage, validRegistration );

			context< CProvideInfoAction >().setResult( validRegistration );
		}
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};


CProvideInfoAction::CProvideInfoAction( uint256 const & _actionKey, uintptr_t _nodeIndicator )
	: common::CScheduleAbleAction< common::CTrackerTypes >( _actionKey )
	, m_nodeIndicator( _nodeIndicator )
{
	initiate();
	process_event( CProvideInfoEvent() );
}

CProvideInfoAction::CProvideInfoAction( common::CInfoKind::Enum _infoKind )
	: m_infoKind( _infoKind )
{
	initiate();
	process_event( CAskForInfoEvent() );
}

void
CProvideInfoAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

uintptr_t
CProvideInfoAction::getNodeIndicator() const
{
	return m_nodeIndicator;
}

}


