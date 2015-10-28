
// Copyright (c) 2014-2015 DiMS dev-team
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
#include "monitor/reputationControlAction.h"

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
					, new CByKeyMediumFilter( _messageResult.m_pubKey ) );

		request->addPayload( common::CAck() );

		context< CProvideInfoAction >().addRequest( request );

		context< CProvideInfoAction >().addRequest(
					new common::CTimeEventRequest(
						  LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );


		if ( requestedInfo.m_kind == (int)common::CInfoKind::IsAddmited )
		{
			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::Result
						, context< CProvideInfoAction >().getActionKey()
						, m_id
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) );

			request->addPayload(
						common::CResult( CReputationTracker::getInstance()->isAddmitedMonitor( context< CProvideInfoAction >().getPartnerKey().GetID() ) ? 1 : 0 ) );

			context< CProvideInfoAction >().addRequest( request );

		}
		else if ( requestedInfo.m_kind == (int)common::CInfoKind::IsRegistered )
		{

			common::CTrackerData trackerData;
			CPubKey monitorPubKey;
			CReputationTracker::getInstance()->checkForTracker( context< CProvideInfoAction >().getPartnerKey().GetID(), trackerData, monitorPubKey );

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::ValidRegistration
						, context< CProvideInfoAction >().getActionKey()
						, m_id
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) );

			request->addPayload(
						common::CValidRegistration(
							monitorPubKey
							, trackerData.m_contractTime
							, trackerData.m_networkTime ) );

			context< CProvideInfoAction >().addRequest( request );

		}
		else if ( requestedInfo.m_kind == (int)common::CInfoKind::RegistrationTermsAsk )
		{
			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::RegistrationTerms
						, context< CProvideInfoAction >().getActionKey()
						, m_id
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) );

			request->addPayload(
						common::CRegistrationTerms( CController::getInstance()->getPrice(), CController::getInstance()->getPeriod() ) );

			context< CProvideInfoAction >().addRequest( request );
		}
		else if ( requestedInfo.m_kind == (int)common::CInfoKind::EnterConditionAsk )
		{
			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::EnterNetworkCondition
						, context< CProvideInfoAction >().getActionKey()
						, m_id
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) );

			request->addPayload(
						common::CEnteranceTerms( CController::getInstance()->getEnterancePrice() ) );

			context< CProvideInfoAction >().addRequest( request );
		}
		else if ( requestedInfo.m_kind == (int)common::CInfoKind::RankingFullInfo )
		{
			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::FullRankingInfo
						, context< CProvideInfoAction >().getActionKey()
						, m_id
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) );

			request->addPayload(
						common::CRankingFullInfo(
							CReputationTracker::getInstance()->getAllyTrackers()
							, CReputationTracker::getInstance()->getAllyMonitors()
							, CReputationTracker::getInstance()->getTrackers()
							, CReputationControlAction::getInstance()->getActionKey() ) );

			context< CProvideInfoAction >().addRequest( request );
		}
		context< CProvideInfoAction >().forgetRequests();
		context< CProvideInfoAction >().setExit();
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
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;

	uint256 m_id;
};

namespace
{
common::CMediumFilter * TargetMediumFilter;
}

struct CAskForInfo : boost::statechart::state< CAskForInfo, CProvideInfoAction >
{
	CAskForInfo( my_context ctx ) : my_base( ctx )
	{
		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, context< CProvideInfoAction >().getActionKey()
					, TargetMediumFilter );

		if ( context< CProvideInfoAction >().getInfo() == (int)common::CInfoKind::BalanceAsk )
		{
			request->addPayload( common::CInfoRequestData( (int)context< CProvideInfoAction >().getInfo(), common::CAuthenticationProvider::getInstance()->getMyKey().GetID() ) );
		}
		else
			request->addPayload( (int)context< CProvideInfoAction >().getInfo(), std::vector<unsigned char>() );

		context< CProvideInfoAction >().addRequest( request );

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

		context< CProvideInfoAction >().setResult( common::CFailureEvent() );
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
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

		if ( ( common::CPayloadKind::Enum )orginalMessage.m_header.m_payloadKind == common::CPayloadKind::Result )
		{
			common::CResult result;

			common::convertPayload( orginalMessage, result );

			context< CProvideInfoAction >().setResult( result );
		}
		else if ( orginalMessage.m_header.m_payloadKind == (int)common::CPayloadKind::Balance )
		{
			common::CBalance balance;
			common::convertPayload( orginalMessage, balance );

			context< CProvideInfoAction >().setResult( common::CAvailableCoinsData( balance.m_availableCoins, balance.m_transactionInputs, uint256() ) );//available  coins  is  not  nice  here
		}
		else if ( orginalMessage.m_header.m_payloadKind == (int)common::CPayloadKind::TrackerInfo )
		{
			common::CTrackerInfo trackerInfo;

			common::convertPayload( orginalMessage, trackerInfo );

			context< CProvideInfoAction >().setResult( trackerInfo );
		}
		else
		{
			context< CProvideInfoAction >().setResult( common::CFailureEvent() );
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

CProvideInfoAction::CProvideInfoAction( uint256 const & _actionKey, CPubKey const & _partnerKey )
	: common::CScheduleAbleAction( _actionKey )
	, m_partnerKey( _partnerKey )
{
	initiate();
	process_event( CProvideInfoEvent() );
}

CProvideInfoAction::CProvideInfoAction( common::CInfoKind::Enum _infoKind, common::CMediumKinds::Enum _mediumKind )
	: m_infoKind( _infoKind )
{
	initiate();

	TargetMediumFilter = new CMediumClassFilter( _mediumKind, 1 );
	process_event( CAskForInfoEvent() );
}

CProvideInfoAction::CProvideInfoAction( common::CInfoKind::Enum _infoKind, CPubKey _pubKey )
	: m_infoKind( _infoKind )
{
	initiate();

	TargetMediumFilter = new CByKeyMediumFilter( _pubKey );
	process_event( CAskForInfoEvent() );
}

void
CProvideInfoAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}


