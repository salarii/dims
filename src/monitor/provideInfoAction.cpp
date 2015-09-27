
// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/events.h"
#include "common/authenticationProvider.h"
#include "common/requests.h"
#include "common/communicationProtocol.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "monitor/provideInfoAction.h"
#include "monitor/monitorRequests.h"
#include "monitor/filters.h"
#include "monitor/reputationTracer.h"
#include "monitor/controller.h"

namespace monitor
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
		context< CProvideInfoAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CInfoRequestData requestedInfo;

		common::convertPayload( orginalMessage, requestedInfo );

		context< CProvideInfoAction >().forgetRequests();

		m_id = _messageResult.m_message.m_header.m_id;

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, context< CProvideInfoAction >().getActionKey()
					, m_id
					, new CSpecificMediumFilter( context< CProvideInfoAction >().getNodeIndicator() ) );

		request->addPayload( common::CAck() );

		context< CProvideInfoAction >().addRequest( request );

		context< CProvideInfoAction >().addRequest(
					new common::CTimeEventRequest(
						  LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );


		if ( requestedInfo.m_kind == (int)common::CInfoKind::IsAddmited )
		{
			uint160 pubKeyId;
			CReputationTracker::getInstance()->getNodeToKey( context< CProvideInfoAction >().getNodeIndicator(), pubKeyId );

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::Result
						, context< CProvideInfoAction >().getActionKey()
						, m_id
						, new CSpecificMediumFilter( context< CProvideInfoAction >().getNodeIndicator() ) );

			request->addPayload(
						common::CResult( CReputationTracker::getInstance()->isAddmitedMonitor( pubKeyId ) ? 1 : 0 ) );

			context< CProvideInfoAction >().addRequest( request );

		}
		else if ( requestedInfo.m_kind == (int)common::CInfoKind::IsRegistered )
		{

			uint160 pubKeyId;
			CReputationTracker::getInstance()->getNodeToKey( context< CProvideInfoAction >().getNodeIndicator(), pubKeyId );

			common::CTrackerData trackerData;
			CPubKey monitorPubKey;
			CReputationTracker::getInstance()->checkForTracker( pubKeyId, trackerData, monitorPubKey );

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::ValidRegistration
						, context< CProvideInfoAction >().getActionKey()
						, m_id
						, new CSpecificMediumFilter( context< CProvideInfoAction >().getNodeIndicator() ) );

			request->addPayload(
						common::CValidRegistration(
							monitorPubKey
							, trackerData.m_contractTime
							, trackerData.m_networkTime ) );

			context< CProvideInfoAction >().addRequest( request );

		}
		else if ( requestedInfo.m_kind == (int)common::CInfoKind::EnterConditionAsk )
		{
			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::EnterNetworkCondition
						, context< CProvideInfoAction >().getActionKey()
						, m_id
						, new CSpecificMediumFilter( context< CProvideInfoAction >().getNodeIndicator() ) );

			request->addPayload(
						common::CEnteranceTerms( CController::getInstance()->getEnterancePrice() ) );

			context< CProvideInfoAction >().addRequest( request );
		}

		context< CProvideInfoAction >().forgetRequests();
		context< CProvideInfoAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAvailableCoinsData const & _availableCoins )
	{
		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::Balance
					, context< CProvideInfoAction >().getActionKey()
					, m_id
					, new CSpecificMediumFilter( context< CProvideInfoAction >().getNodeIndicator() ) );

		request->addPayload( common::CBalance( _availableCoins.m_availableCoins ) );

		context< CProvideInfoAction >().addRequest( request );

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CProvideInfoAction >().forgetRequests();
		context< CProvideInfoAction >().setExit();

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CAvailableCoinsData >,
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;

	uint256 m_id;
};

struct CAskForInfo : boost::statechart::state< CAskForInfo, CProvideInfoAction >
{
	CAskForInfo( my_context ctx ) : my_base( ctx )
	{
		context< CProvideInfoAction >().addRequest( new common::CInfoAskRequest(
														  context< CProvideInfoAction >().getInfo()
														, context< CProvideInfoAction >().getActionKey()
														, new CMediumClassFilter( common::CMediumKinds::Monitors, 1 ) ) );

		context< CProvideInfoAction >().addRequest(
					new common::CTimeEventRequest(
						  LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CProvideInfoAction >().forgetRequests();
		context< CProvideInfoAction >().setExit();

		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{

		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		context< CProvideInfoAction >().addRequest(
					new common::CAckRequest(
						  context< CProvideInfoAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

		if ( ( common::CPayloadKind::Enum )orginalMessage.m_header.m_payloadKind == common::CPayloadKind::Result )
		{
			common::CResult result;

			common::convertPayload( orginalMessage, result );

			context< CProvideInfoAction >().setResult( result );
		}

		context< CProvideInfoAction >().forgetRequests();
		context< CProvideInfoAction >().setExit();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CMonitorStop : boost::statechart::state< CMonitorStop, CProvideInfoAction >
{
	CMonitorStop( my_context ctx ) : my_base( ctx )
	{
	}
};

CProvideInfoAction::CProvideInfoAction( uint256 const & _actionKey, uintptr_t _nodeIndicator )
	: common::CScheduleAbleAction( _actionKey )
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
CProvideInfoAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

uintptr_t
CProvideInfoAction::getNodeIndicator() const
{
	return m_nodeIndicator;
}

}


