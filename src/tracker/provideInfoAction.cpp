// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectNodeAction.h"
#include "common/setResponseVisitor.h"
#include "common/events.h"
#include "common/authenticationProvider.h"
#include "common/requests.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "tracker/provideInfoAction.h"
#include "tracker/requests.h"
#include "tracker/controller.h"

namespace tracker
{

unsigned int const LoopTime = 20000;//milisec

struct CProvideInfo;
struct CAskForInfo;

struct CAskForInfoEvent : boost::statechart::event< CAskForInfoEvent >{};

struct CProvideInfoEvent : boost::statechart::event< CProvideInfoEvent >{};

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

		context< CProvideInfoAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, common::CAck()
					, context< CProvideInfoAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CByKeyMediumFilter( context< CProvideInfoAction >().getPartnerKey() ) ) );

		if ( requestedInfo.m_kind == (int)common::CInfoKind::BalanceAsk )
		{
			CKeyID keyId;
			common::readPayload( requestedInfo.m_payload, keyId );

			context< CProvideInfoAction >().addRequest(
						new CGetBalanceRequest( keyId ) );
		}
		if ( requestedInfo.m_kind == (int)common::CInfoKind::TrackerInfo )
		{
			common::CTrackerInfo trackerInfo(
				common::CAuthenticationProvider::getInstance()->getMyKey()
				, CController::getInstance()->getPrice() );

			context< CProvideInfoAction >().addRequest(
					new common::CSendMessageRequest(
						common::CPayloadKind::TrackerInfo
						, trackerInfo
						, context< CProvideInfoAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CByKeyMediumFilter( context< CProvideInfoAction >().getPartnerKey() ) ) );
		}

		return discard_event();
	}


	boost::statechart::result react( common::CAvailableCoinsData const & _availableCoins )
	{
		context< CProvideInfoAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::Balance
					, common::CBalance( _availableCoins.m_availableCoins, _availableCoins.m_transactionInputs )
					, context< CProvideInfoAction >().getActionKey()
					, m_id
					, new CByKeyMediumFilter( context< CProvideInfoAction >().getPartnerKey() ) ) );

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
		context< CProvideInfoAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, common::CInfoRequestData( (int)context< CProvideInfoAction >().getInfo(), std::vector<unsigned char>() )
					, context< CProvideInfoAction >().getActionKey()
					, context< CProvideInfoAction >().m_targetMediumFilter ) );

		if ( common::CInfoKind::RankingFullInfo == context< CProvideInfoAction >().getInfo() )
		{
			context< CProvideInfoAction >().setExit();// ugly but this will allow CValidateTransactionsAction handle this
		}

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

		if ( ( common::CPayloadKind::Enum )orginalMessage.m_header.m_payloadKind == common::CPayloadKind::ValidRegistration )
		{
			common::CValidRegistration validRegistration;

			common::convertPayload( orginalMessage, validRegistration );

			context< CProvideInfoAction >().setResult( validRegistration );
		}
		else if ( ( common::CPayloadKind::Enum )orginalMessage.m_header.m_payloadKind == common::CPayloadKind::RegistrationTerms )
		{
			common::CRegistrationTerms registrationTerms;

			common::convertPayload( orginalMessage, registrationTerms );

			context< CProvideInfoAction >().setResult( registrationTerms );
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


CProvideInfoAction::CProvideInfoAction( uint256 const & _actionKey, CPubKey const & _partnerKey )
	: common::CScheduleAbleAction( _actionKey )
	, m_partnerKey( _partnerKey )
	, m_targetMediumFilter( new CByKeyMediumFilter( _partnerKey ) )
{
	LogPrintf("provide info action: %p provide \n", this );

	initiate();
	process_event( CProvideInfoEvent() );
}

CProvideInfoAction::CProvideInfoAction( common::CInfoKind::Enum _infoKind, CPubKey const & _partnerKey )
	: m_infoKind( _infoKind )
	, m_partnerKey( _partnerKey )
	, m_targetMediumFilter( new CByKeyMediumFilter( _partnerKey ) )
{
	LogPrintf("provide info action: %p ask \n", this );

	initiate();
	process_event( CAskForInfoEvent() );
}

CProvideInfoAction::CProvideInfoAction( common::CInfoKind::Enum _infoKind, common::CMediumKinds::Enum _mediumKind )
	: m_infoKind( _infoKind )
	, m_targetMediumFilter( new CMediumClassFilter( _mediumKind, 1 ) )
{
	LogPrintf("provide info action: %p ask \n", this );

	initiate();

	process_event( CAskForInfoEvent() );
}

void
CProvideInfoAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}


