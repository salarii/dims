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

			context< CRegisterAction >().addRequest(
					new common::CSendMessageRequest(
						common::CPayloadKind::Ack
						, common::CAck()
						, context< CRegisterAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) ) );

			if ( CController::getInstance()->autoRenewRegistration() )
			{
				context< CRegisterAction >().addRequest(
							new common::CSendMessageRequest(
								common::CPayloadKind::AdmitAsk
								, common::CAdmitAsk()
								, context< CRegisterAction >().getActionKey()
								, _messageResult.m_message.m_header.m_id
								, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) ) );

				m_price = connectCondition.m_price;
			}
		}
		else if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::Result )
		{
			common::CResult result;

			common::convertPayload( orginalMessage, result );

			context< CRegisterAction >().addRequest(
						new common::CAckRequest(
							context< CRegisterAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

			if ( !result.m_result )
			{
				CTrackerNodesManager::getInstance()->setMyMonitor( CKeyID() );
			}
		}

			return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CRegisterAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		if ( m_price )
			transit< CNoTrackers >();

		context< CRegisterAction >().setExit();
		return discard_event();
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

		context< CRegisterAction >().addRequest(
					new common::CSendMessageRequest(
						common::CPayloadKind::AdmitAsk
					, common::CAdmitAsk()
					, context< CRegisterAction >().getActionKey()
						, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) ) );

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
			CTrackerNodesManager::getInstance()->setMyMonitor( context< CRegisterAction >().getPartnerKey().GetID());
			context< CRegisterAction >().setInNetwork( true );
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

		context< CRegisterAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, common::CAck()
					, context< CRegisterAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) ) );

		CTrackerNodesManager::getInstance()->setMyMonitor( context< CRegisterAction >().getPartnerKey().GetID());
		context< CRegisterAction >().setInNetwork( true );
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

			context< CRegisterAction >().addRequest(
					new common::CSendMessageRequest(
						common::CPayloadKind::AdmitProof
						, admitProof
						, context< CRegisterAction >().getActionKey()
						, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) ) );

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

		context< CRegisterAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::Ack
					, common::CAck()
					, context< CRegisterAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) ) );

		context< CRegisterAction >().forgetRequests();

		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest(
						new CSynchronizationAction( context< CRegisterAction >().getPartnerKey() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );

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

			context< CRegisterAction >().addRequest(
						new common::CSendMessageRequest(
							common::CPayloadKind::AdmitProof
							, admitProof
							, context< CRegisterAction >().getActionKey()
							, new CByKeyMediumFilter( context< CRegisterAction >().getPartnerKey() ) ) );

		}
		return discard_event();
	}

	boost::statechart::result react( common::CSynchronizationResult const & _synchronizationResult )
	{
		if ( _synchronizationResult.m_result )
		{
			CTrackerNodesManager::getInstance()->setMyMonitor( context< CRegisterAction >().getPartnerKey().GetID());
			context< CRegisterAction >().setInNetwork( true );
		}
		else
		{
			assert(!"problem");
		}
		context< CRegisterAction >().setExit();
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
	boost::statechart::custom_reaction< common::CSynchronizationResult >,
	boost::statechart::custom_reaction< common::CFailureEvent >
	> reactions;
};

CRegisterAction::CRegisterAction( uint256 const & _actionKey, CPubKey const & _partnerKey )
	: common::CAction( _actionKey )
	, m_partnerKey( _partnerKey )
{
	LogPrintf("register action: %p extend \n", this );

	initiate();
	process_event( CExtensionEvent() );
}

CRegisterAction::CRegisterAction( CPubKey const & _partnerKey )
	: m_partnerKey( _partnerKey )
{
	LogPrintf("register action: %p register \n", this );

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

			CController::getInstance()->process_event( CMonitorAcceptEvent( m_partnerKey ) );
			common::CValidNodeInfo validNodeInfo;

			if ( !CTrackerNodesManager::getInstance()->getNodeInfo( m_partnerKey.GetID(), validNodeInfo ) )
				assert( !"problem" );

			CTrackerNodesManager::getInstance()->setNetworkMonitor( validNodeInfo );

			CTrackerNodesManager::getInstance()->setActiveNode( m_partnerKey.GetID() );
	}
}

}
