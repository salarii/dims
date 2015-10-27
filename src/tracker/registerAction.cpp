// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "registerAction.h"
#include "wallet.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/requests.h"
#include "common/setResponseVisitor.h"

#include "tracker/passTransactionAction.h"
#include "tracker/requests.h"
#include "tracker/synchronizationAction.h"
#include "tracker/getBalanceAction.h"
#include "tracker/controller.h"
#include "tracker/controllerEvents.h"
#include "tracker/provideInfoAction.h"

namespace tracker
{

//milisec
unsigned int const WaitTime = 20000;
unsigned int const MoneyWaitTime = 30000;

struct CFreeRegistration;
struct CSynchronize;
struct CNoTrackers;
struct COriginateRegistration;
struct CInitiateRegistration;
struct CRegistrationExtension;
struct CNetworkAlive;

struct CExtensionEvent : boost::statechart::event< CExtensionEvent >{};
struct CNewEvent : boost::statechart::event< CNewEvent >{};

struct CInitiateRegistration : boost::statechart::simple_state< CInitiateRegistration, CRegisterAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CExtensionEvent, CRegistrationExtension >,
	boost::statechart::transition< CNewEvent, COriginateRegistration >
	> reactions;
};

struct CRegistrationExtension : boost::statechart::state< CRegistrationExtension, CRegisterAction >
{

	CRegistrationExtension( my_context ctx )
		: my_base( ctx )
	{}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::ExtendRegistration )
		{
			common::CRegistrationTerms connectCondition;

			common::convertPayload( orginalMessage, connectCondition );

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::Ack
						, context< CRegisterAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) );

			request->addPayload( common::CAck() );

			context< CRegisterAction >().addRequest( request );

			if ( CController::getInstance()->autoRenewRegistration() )
			{
				request = new common::CSendMessageRequest(
							common::CPayloadKind::AdmitAsk
							, context< CRegisterAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) );

				request->addPayload( common::CAdmitAsk() );
				context< CRegisterAction >().addRequest( request );

				m_price = connectCondition.m_price;
			}
		}
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return m_price ? transit< CNoTrackers >() : transit< CFreeRegistration >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	unsigned int m_price;
};

struct COriginateRegistration : boost::statechart::state< COriginateRegistration, CRegisterAction >
{
	COriginateRegistration( my_context ctx )
		: my_base( ctx )
	{
		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest(
						new CProvideInfoAction( common::CInfoKind::RegistrationTermsAsk, context< CRegisterAction >().getPartnerKey() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CResult result;

		common::convertPayload( orginalMessage, result );

		context< CRegisterAction >().addRequest(
					new common::CAckRequest(
						context< CRegisterAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) ) );

		if ( result.m_result )
		{
		if ( !m_registrationTerms.m_price )
		{
			return transit< CFreeRegistration >();
		}
		else
		{
			common::CNetworkRecognizedData networkData =
					CController::getInstance()->getNetworkData();

			context< CRegisterAction >().setRegisterPayment( m_registrationTerms.m_price );

			return networkData.m_trackersInfo.empty()
					? transit< CSynchronize >()
					: transit< CNetworkAlive >();
		}
		}
		else
		{
			context< CRegisterAction >().setExit();
			return discard_event();
		}
	}

	boost::statechart::result react( common::CRegistrationTermsEvent const & _registrationTermsEvent )
	{
		m_registrationTerms = _registrationTermsEvent.m_registrationTerms;

		LogPrintf("register action: %p initiate registration \n", &context< CRegisterAction >() );
		context< CRegisterAction >().forgetRequests();
		context< CRegisterAction >().addRequest(
		 new common::CTimeEventRequest(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::AdmitAsk
						, context< CRegisterAction >().getActionKey()
						, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) );

		request->addPayload( common::CAdmitAsk() );

		context< CRegisterAction >().addRequest( request );

		return discard_event();
	}

	boost::statechart::result react( common::CFailureEvent const & _failureEvent )

	{
		context< CRegisterAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CRegisterAction >().forgetRequests();
		context< CRegisterAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CRegistrationTermsEvent >,
	boost::statechart::custom_reaction< common::CFailureEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	common::CRegistrationTerms m_registrationTerms;
};

struct CFreeRegistration : boost::statechart::state< CFreeRegistration, CRegisterAction >
{
	CFreeRegistration( my_context ctx )
		: my_base( ctx )
	{
		LogPrintf("register action: %p free registration \n", &context< CRegisterAction >() );

		context< CRegisterAction >().forgetRequests();

		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest(
						new CSynchronizationAction( context< CRegisterAction >().getPartnerKey() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );

	}

	boost::statechart::result react( common::CSynchronizationResult const & _synchronizationResult )
	{
		if ( _synchronizationResult.m_result )
		{
			CController::getInstance()->setConnected( true );
		}
		context< CRegisterAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CFailureEvent const & _failureEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CSynchronizationResult >,
	boost::statechart::custom_reaction< common::CFailureEvent >
	> reactions;
};


struct CSynchronize : boost::statechart::state< CSynchronize, CRegisterAction >
{
	CSynchronize( my_context ctx )
		: my_base( ctx )
	{
		context< CRegisterAction >().forgetRequests();

		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest(
						new CSynchronizationAction( context< CRegisterAction >().getPartnerKey() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CSynchronizationResult const & _synchronizationResult )
	{
		if ( _synchronizationResult.m_result )
		{
			return transit< CNoTrackers >();
		}

		return discard_event();
	}

	boost::statechart::result react( common::CFailureEvent const & _failureEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CSynchronizationResult >,
	boost::statechart::custom_reaction< common::CFailureEvent >
	> reactions;
};
//stupid  name
struct CNoTrackers : boost::statechart::state< CNoTrackers, CRegisterAction >
{
	// send  ready  to  monitor      ??????
	CNoTrackers( my_context ctx )
		: my_base( ctx )
	{

		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest(
						new CPassTransactionAction(
							context< CRegisterAction >().getPartnerKey().GetID()
							, context< CRegisterAction >().getRegisterPayment() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CResult result;

		common::convertPayload( orginalMessage, result );

		assert( result.m_result );// for debug only, do something here

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, context< CRegisterAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) );

		request->addPayload( common::CAck() );

		context< CRegisterAction >().addRequest( request );

		// registration done
		CController::getInstance()->process_event( CMonitorAcceptEvent( _messageResult.m_pubKey ) );

		return discard_event();
	}

	boost::statechart::result react( common::CTransactionAckEvent const & _transactionAckEvent )
	{
		if ( _transactionAckEvent.m_status == common::TransactionsStatus::Invalid )
		{
			context< CRegisterAction >().addRequest(
						new common::CTimeEventRequest(
							MoneyWaitTime
							, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
		}
		else
		{
			common::CAdmitProof admitProof;
			admitProof.m_proofTransactionHash = _transactionAckEvent.m_transactionSend.GetHash();

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::AdmitProof
						, context< CRegisterAction >().getActionKey()
						, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) );

			request->addPayload(admitProof);

			context< CRegisterAction >().addRequest( request );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest(
						new CPassTransactionAction(
							context< CRegisterAction >().getPartnerKey().GetID()
							, context< CRegisterAction >().getRegisterPayment() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CFailureEvent const & _failureEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTransactionAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CFailureEvent >
	> reactions;
};


struct CNetworkAlive : boost::statechart::state< CNetworkAlive, CRegisterAction >
{
	CNetworkAlive( my_context ctx )
		: my_base( ctx )
	{
		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest(
						new CPassTransactionAction(
							context< CRegisterAction >().getPartnerKey().GetID()
							, context< CRegisterAction >().getRegisterPayment() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CResult result;

		common::convertPayload( orginalMessage, result );

		assert( result.m_result );// for debug only, do something here

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, context< CRegisterAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) );

		request->addPayload( common::CAck() );

		context< CRegisterAction >().addRequest( request );

		// registration done
		CController::getInstance()->process_event( common::CRegistrationData( _messageResult.m_pubKey, GetTime(), 0 ) );

		context< CRegisterAction >().setExit();

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CRegisterAction >().forgetRequests();
		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest(
						new CPassTransactionAction(
							context< CRegisterAction >().getPartnerKey().GetID()
							, context< CRegisterAction >().getRegisterPayment() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CTransactionAckEvent const & _transactionAckEvent )
	{
		if ( _transactionAckEvent.m_status == common::TransactionsStatus::Invalid )
		{
			context< CRegisterAction >().addRequest(
						new common::CTimeEventRequest(
							MoneyWaitTime
							, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
		}
		else
		{
			common::CAdmitProof admitProof;
			admitProof.m_proofTransactionHash = _transactionAckEvent.m_transactionSend.GetHash();

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::AdmitProof
						, context< CRegisterAction >().getActionKey()
						, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) );

			request->addPayload(admitProof);

			context< CRegisterAction >().addRequest( request );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CFailureEvent const & _failureEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTransactionAckEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CFailureEvent >
	> reactions;
};

CRegisterAction::CRegisterAction( uint256 const & _actionKey, CPubKey const & _partnerKey )
	: common::CAction( _actionKey )
	, m_partnerKey( _partnerKey )
{
	initiate();
	process_event( CExtensionEvent() );
}

CRegisterAction::CRegisterAction( CPubKey const & _partnerKey )
	: m_partnerKey( _partnerKey )
{
	initiate();
	process_event( CNewEvent() );
}

void
CRegisterAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

void
CRegisterAction::setInNetwork( bool _flag )
{
	CController::getInstance()->setConnected( _flag );
	if ( _flag )
	{
			CController::getInstance()->process_event( common::CRegistrationData( m_partnerKey, GetTime(), 0 ) );

			common::CValidNodeInfo validNodeInfo;

			if ( !CTrackerNodesManager::getInstance()->getNodeInfo( m_partnerKey.GetID(), validNodeInfo ) )
				assert( !"problem" );

			CTrackerNodesManager::getInstance()->setNetworkMonitor( validNodeInfo );
	}
}

}
