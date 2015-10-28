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

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, context< CProvideInfoAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CByKeyMediumFilter( context< CProvideInfoAction >().getPartnerKey() ) );

		request->addPayload( common::CAck() );

		context< CProvideInfoAction >().addRequest( request );

		if ( requestedInfo.m_kind == (int)common::CInfoKind::BalanceAsk )
		{
			CKeyID keyId;
			common::readPayload( requestedInfo.m_payload, keyId );

			context< CProvideInfoAction >().addRequest(
						new CGetBalanceRequest( keyId ) );
		}
		if ( requestedInfo.m_kind == (int)common::CInfoKind::TrackerInfo )
		{
			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::TrackerInfo
						, context< CProvideInfoAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CByKeyMediumFilter( context< CProvideInfoAction >().getPartnerKey() ) );

			request->addPayload(
						common::CTrackerInfo(
							common::CAuthenticationProvider::getInstance()->getMyKey()
							, CController::getInstance()->getPrice() ) );

			context< CProvideInfoAction >().addRequest( request );

		}

		return discard_event();
	}


	boost::statechart::result react( common::CAvailableCoinsData const & _availableCoins )
	{
		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::Balance
					, context< CProvideInfoAction >().getActionKey()
					, m_id
					, new CByKeyMediumFilter( context< CProvideInfoAction >().getPartnerKey() ) );

		request->addPayload( common::CBalance( _availableCoins.m_availableCoins, _availableCoins.m_transactionInputs ) );

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

		request->addPayload( common::CInfoRequestData( (int)context< CProvideInfoAction >().getInfo(), std::vector<unsigned char>() ) );

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
{
	TargetMediumFilter = new CByKeyMediumFilter( _partnerKey );
	initiate();
	process_event( CProvideInfoEvent() );
}

CProvideInfoAction::CProvideInfoAction( common::CInfoKind::Enum _infoKind, CPubKey const & _partnerKey )
	: m_infoKind( _infoKind )
	, m_partnerKey( _partnerKey )
{
	TargetMediumFilter = new CByKeyMediumFilter( _partnerKey );
	initiate();
	process_event( CAskForInfoEvent() );
}

CProvideInfoAction::CProvideInfoAction( common::CInfoKind::Enum _infoKind, common::CMediumKinds::Enum _mediumKind )
	: m_infoKind( _infoKind )
{
	initiate();

	TargetMediumFilter = new CMediumClassFilter( _mediumKind, 1 );
	process_event( CAskForInfoEvent() );
}

void
CProvideInfoAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}


