// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/authenticationProvider.h"
#include "common/setResponseVisitor.h"
#include "common/analyseTransaction.h"
#include "common/requests.h"
#include "common/events.h"

#include "monitor/admitTrackerAction.h"
#include "monitor/controller.h"
#include "monitor/admitTransactionsBundle.h"
#include "monitor/filters.h"
#include "monitor/reputationTracer.h"
#include "monitor/monitorRequests.h"
#include "monitor/rankingDatabase.h"
#include "monitor/chargeRegister.h"

namespace monitor
{
struct CPaidRegistration;
struct CFreeRegistration;
struct CPaidRegistrationEmptyNetwork;
struct CWaitForInfo;
struct CExtendRegistration;
//milisec
unsigned int const WaitTime = 20000;

struct CRegisterEvent : boost::statechart::event< CRegisterEvent >{};

struct CExtendEvent : boost::statechart::event< CExtendEvent >{};

struct CAdmitInitial : boost::statechart::simple_state< CAdmitInitial, CAdmitTrackerAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CRegisterEvent, CWaitForInfo >,
	boost::statechart::transition< CExtendEvent, CExtendRegistration >
	> reactions;
};

struct CExtendRegistration : boost::statechart::state< CExtendRegistration, CAdmitTrackerAction >
{
	CExtendRegistration( my_context ctx )
		: my_base( ctx )
	{
		LogPrintf("admit tracker action: %p extend registration \n", &context< CAdmitTrackerAction >() );

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::ExtendRegistration
					, context< CAdmitTrackerAction >().getActionKey()
					, new CSpecificMediumFilter( context< CAdmitTrackerAction >().getNodePtr() ) );

		common::CRegistrationTerms registrationTerms(
					CController::getInstance()->getPrice()
					, CController::getInstance()->getPeriod() );

		request->addPayload( registrationTerms );

		context< CAdmitTrackerAction >().addRequest( request );

	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::AdmitAsk )
		{
			context< CAdmitTrackerAction >().forgetRequests();

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::Ack
						, context< CAdmitTrackerAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CAdmitTrackerAction >().getNodePtr() ) );

			request->addPayload( common::CAck() );

			context< CAdmitTrackerAction >().addRequest( request );

			return transit< CPaidRegistration >();
		}

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CAdmitTrackerAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

struct CWaitForInfo : boost::statechart::state< CWaitForInfo, CAdmitTrackerAction >
{
	CWaitForInfo( my_context ctx )
		: my_base( ctx )
	{
		LogPrintf("admit tracker action: %p wait for info \n", &context< CAdmitTrackerAction >() );
		context< CAdmitTrackerAction >().forgetRequests();
		context< CAdmitTrackerAction >().addRequest( new common::CTimeEventRequest( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		// todo create alredy registered logic _messageResult.m_pubKey

		context< CAdmitTrackerAction >().forgetRequests();

		context< CAdmitTrackerAction >().addRequest(
					new common::CAckRequest(
						context< CAdmitTrackerAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CAdmitTrackerAction >().getNodePtr() ) ) );

		context< CAdmitTrackerAction >().addRequest( new CRegistrationTerms(
														  CController::getInstance()->getPrice()
														 , CController::getInstance()->getPeriod()
														 , context< CAdmitTrackerAction >().getActionKey()
														 , _messageResult.m_message.m_header.m_id
														 , new CSpecificMediumFilter( context< CAdmitTrackerAction >().getNodePtr() ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		if ( CController::getInstance()->getPrice() )
		{
			if ( CReputationTracker::getInstance()->getTrackers().empty() )
				return transit< CPaidRegistrationEmptyNetwork >();
		}
		else
			return transit< CFreeRegistration >();

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CAdmitTrackerAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

struct CFreeRegistration : boost::statechart::state< CFreeRegistration, CAdmitTrackerAction >
{
	CFreeRegistration( my_context ctx )
		: my_base( ctx )
	{

		LogPrintf("admit tracker action: %p free registration \n", &context< CAdmitTrackerAction >() );

		context< CAdmitTrackerAction >().forgetRequests();
		context< CAdmitTrackerAction >().addRequest( new common::CTimeEventRequest( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CAdmitProof admitMessage;

		common::convertPayload( orginalMessage, admitMessage );

		CReputationTracker::getInstance()->addTracker( common::CTrackerData( _messageResult.m_pubKey, 0, CController::getInstance()->getPeriod(), GetTime() ) );

		context< CAdmitTrackerAction >().addRequest(
					new common::CAckRequest(
						context< CAdmitTrackerAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CAdmitTrackerAction >().getNodePtr() ) ) );

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::Result
					, context< CAdmitTrackerAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CSpecificMediumFilter( context< CAdmitTrackerAction >().getNodePtr() ) );

		request->addPayload( common::CResult( 1 ) );

		context< CAdmitTrackerAction >().addRequest( request );

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CAdmitTrackerAction >().forgetRequests();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CAdmitTrackerAction >().forgetRequests();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

};

struct CPaidRegistrationEmptyNetwork : boost::statechart::state< CPaidRegistrationEmptyNetwork, CAdmitTrackerAction >
{
	CPaidRegistrationEmptyNetwork( my_context ctx )
		: my_base( ctx )
	{
		context< CAdmitTrackerAction >().forgetRequests();
		context< CAdmitTrackerAction >().addRequest(
					new common::CTimeEventRequest(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( _messageResult.m_message.m_header.m_payloadKind== common::CPayloadKind::AdmitAsk )
		{
			context< CAdmitTrackerAction >().addRequest(
						new common::CAckRequest(
							context< CAdmitTrackerAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CAdmitTrackerAction >().getNodePtr() ) ) );

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::Result
						, context< CAdmitTrackerAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CAdmitTrackerAction >().getNodePtr() ) );

			request->addPayload( common::CResult( 1 ) );

			context< CAdmitTrackerAction >().addRequest( request );

			CReputationTracker::getInstance()->addTracker( common::CTrackerData( _messageResult.m_pubKey, 0, CController::getInstance()->getPeriod(), GetTime() ) );

		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CAdmitTrackerAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return transit< CPaidRegistration >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

struct CPaidRegistration : boost::statechart::state< CPaidRegistration, CAdmitTrackerAction >
{
	CPaidRegistration( my_context ctx )
		: my_base( ctx )
		, m_checkPeriod( 30000 )
	{
		LogPrintf("admit tracker action: %p paid registration \n", &context< CAdmitTrackerAction >() );
		context< CAdmitTrackerAction >().forgetRequests();

		CChargeRegister::getInstance()->setStoreTransactions( true );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CAdmitProof admitMessage;

		common::convertPayload( orginalMessage, admitMessage );

		CChargeRegister::getInstance()->addTransactionToSearch( admitMessage.m_proofTransactionHash, _messageResult.m_pubKey.GetID() );

		m_proofHash = admitMessage.m_proofTransactionHash;

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, context< CAdmitTrackerAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CSpecificMediumFilter( context< CAdmitTrackerAction >().getNodePtr() ) );

		request->addPayload( common::CAck() );

		context< CAdmitTrackerAction >().addRequest( request );

		context< CAdmitTrackerAction >().forgetRequests();
		context< CAdmitTrackerAction >().addRequest(
					new common::CTimeEventRequest(
						m_checkPeriod
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		m_messageId = _messageResult.m_message.m_header.m_id;

		m_pubKey = _messageResult.m_pubKey;

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		CTransaction transaction;

		if ( CChargeRegister::getInstance()->isTransactionPresent( m_proofHash ) )
		{

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::Result
						, context< CAdmitTrackerAction >().getActionKey()
						, m_messageId
						, new CSpecificMediumFilter( context< CAdmitTrackerAction >().getNodePtr() ) );

			CPubKey pubKey;

			if ( CReputationTracker::getInstance()->getNodeToKey( context< CAdmitTrackerAction >().getNodePtr(), pubKey ) )
			{
				request->addPayload( common::CResult( 1 ) );

							common::CTrackerData trackerData(
								m_pubKey
								, 0
								, CController::getInstance()->getPeriod()
								, GetTime() );

				CRankingDatabase::getInstance()->writeTrackerData( trackerData );

				CReputationTracker::getInstance()->addTracker( trackerData );
			}
			else
			{
				request->addPayload( common::CResult( 0 ) );
			}

			context< CAdmitTrackerAction >().addRequest( request );
		}
		else
		{
			context< CAdmitTrackerAction >().forgetRequests();

			context< CAdmitTrackerAction >().addRequest(
						new common::CTimeEventRequest(
							m_checkPeriod
							, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		}
		return discard_event();
	}

	~CPaidRegistration()
	{
		CChargeRegister::getInstance()->setStoreTransactions( false );
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CAdmitTrackerAction >().setExit();
		return discard_event();
	}


	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

	uint256 m_proofHash;

	uint256 m_messageId;

	int64_t const m_checkPeriod;

	CPubKey m_pubKey;
};

CAdmitTrackerAction::CAdmitTrackerAction( uintptr_t _nodePtr )
	: m_nodePtr( _nodePtr )
{
	initiate();
	process_event( CExtendEvent() );
}

CAdmitTrackerAction::CAdmitTrackerAction( uint256 const & _actionKey, uintptr_t _nodePtr )
	: common::CAction( _actionKey )
	, m_nodePtr( _nodePtr )
{
	initiate();
	process_event( CRegisterEvent() );
}

void
CAdmitTrackerAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}
