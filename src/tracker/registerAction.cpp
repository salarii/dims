// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "registerAction.h"
#include "wallet.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/commonRequests.h"
#include "common/setResponseVisitor.h"

#include "tracker/passTransactionAction.h"
#include "tracker/requests.h"
#include "tracker/selfWallet.h"
#include "tracker/synchronizationAction.h"
#include "tracker/getBalanceAction.h"
#include "tracker/controller.h"
#include "tracker/controllerEvents.h"

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

			common::CSendMessageRequest< common::CTrackerTypes > * request =
					new common::CSendMessageRequest< common::CTrackerTypes >(
						common::CPayloadKind::Ack
						, context< CRegisterAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) );

			request->addPayload( common::CAck() );

			context< CRegisterAction >().addRequest( request );

			if ( CController::getInstance()->autoRenewRegistration() )
			{
				request = new common::CSendMessageRequest< common::CTrackerTypes >(
							common::CPayloadKind::AdmitAsk
							, context< CRegisterAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) );

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
		LogPrintf("register action: %p initiate registration \n", &context< CRegisterAction >() );
		context< CRegisterAction >().forgetRequests();
		context< CRegisterAction >().addRequest(
		 new common::CTimeEventRequest< common::CTrackerTypes >(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CRegisterAction >().addRequest(
					new CAskForRegistrationRequest(
						context< CRegisterAction >().getActionKey()
						, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CRegistrationTerms connectCondition;

		common::convertPayload( orginalMessage, connectCondition );

		context< CRegisterAction >().addRequest(
					new common::CAckRequest< common::CTrackerTypes >(
						context< CRegisterAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) ) );

		if ( !connectCondition.m_price )
		{
			return transit< CFreeRegistration >();
		}
		else
		{
			context< CRegisterAction >().setRegisterPayment( connectCondition.m_price );
			return transit< CSynchronize >();
		}
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		assert( !"no response" );
		context< CRegisterAction >().forgetRequests();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CFreeRegistration : boost::statechart::state< CFreeRegistration, CRegisterAction >
{
	CFreeRegistration( my_context ctx )
		: my_base( ctx )
	{
		LogPrintf("register action: %p free registration \n", &context< CRegisterAction >() );

		context< CRegisterAction >().forgetRequests();

		context< CRegisterAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CRegisterAction >().addRequest(
					new CRegisterProofRequest(
						uint256()
						, context< CRegisterAction >().getActionKey()
						, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) ) );
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
					new common::CAckRequest< common::CTrackerTypes >(
						  context< CRegisterAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CRegisterAction >().forgetRequests();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};


struct CSynchronize : boost::statechart::state< CSynchronize, CRegisterAction >
{
	CSynchronize( my_context ctx )
		: my_base( ctx )
	{
		context< CRegisterAction >().forgetRequests();
		context< CRegisterAction >().addRequest(
		 new common::CTimeEventRequest< common::CTrackerTypes >(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CRegisterAction >().addRequest(
					new CAskForRegistrationRequest(
						context< CRegisterAction >().getActionKey()
						, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest< common::CTrackerTypes >(
						new CSynchronizationAction( context< CRegisterAction >().getNodePtr() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CResult result;

		common::convertPayload( orginalMessage, result );

		assert( result.m_result );// for debug only, do something here

		CController::getInstance()->setConnected( true );

		context< CRegisterAction >().addRequest(
					new common::CAckRequest< common::CTrackerTypes >(
						  context< CRegisterAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CSynchronizationResult const & _synchronizationResult )
	{
		if ( _synchronizationResult.m_result )
		{
			return transit< CNoTrackers >();
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CSynchronizationResult >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
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
					new common::CScheduleActionRequest< common::CTrackerTypes >(
						new CPassTransactionAction(
							context< CRegisterAction >().getPublicKey().GetID()
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

		common::CSendMessageRequest< common::CTrackerTypes > * request =
				new common::CSendMessageRequest< common::CTrackerTypes >(
					common::CPayloadKind::Ack
					, context< CRegisterAction >().getActionKey()
					, _messageResult.m_message.m_header.m_id
					, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) );

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
						new common::CTimeEventRequest< common::CTrackerTypes >(
							MoneyWaitTime
							, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
		}
		else
		{
			common::CAdmitProof admitProof;
			admitProof.m_proofTransactionHash = _transactionAckEvent.m_transactionSend.GetHash();

			common::CSendMessageRequest< common::CTrackerTypes > * request =
					new common::CSendMessageRequest< common::CTrackerTypes >(
						common::CPayloadKind::AdmitProof
						, context< CRegisterAction >().getActionKey()
						, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) );

			request->addPayload(admitProof);

			context< CRegisterAction >().addRequest( request );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest< common::CTrackerTypes >(
						new CPassTransactionAction(
							context< CRegisterAction >().getPublicKey().GetID()
							, context< CRegisterAction >().getRegisterPayment() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CRegisterAction >().forgetRequests();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTransactionAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};


struct CNetworkAlive : boost::statechart::state< CNetworkAlive, CRegisterAction >
{
	// follow ------>> 2 <<-------
	CNetworkAlive( my_context ctx )
		: my_base( ctx )
	{
		context< CRegisterAction >().forgetRequests();
		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest< common::CTrackerTypes >(
						new CGetBalanceAction()
						, new CMediumClassFilter( common::CMediumKinds::Schedule ) ) );
	}

	boost::statechart::result react( common::CExecutedIndicator const & _executedIndicator )
	{
		context< CRegisterAction >().forgetRequests();

		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest< common::CTrackerTypes >(
						new CPassTransactionAction(
							context< CRegisterAction >().getPublicKey().GetID()
							, context< CRegisterAction >().getRegisterPayment() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CExecutedIndicator >
	> reactions;
};

CRegisterAction::CRegisterAction( uint256 const & _actionKey, uintptr_t _nodePtr )
	: common::CAction< common::CTrackerTypes >( _actionKey )
	, m_nodePtr( _nodePtr )
{
	initiate();
	process_event( CExtensionEvent() );
}

CRegisterAction::CRegisterAction( uintptr_t _nodePtr )
	: m_nodePtr( _nodePtr )
{
	initiate();
	process_event( CNewEvent() );
}

CPubKey
CRegisterAction::getPublicKey() const
{
	CAddress address;
	if ( !CTrackerNodesManager::getInstance()->getAddress( m_nodePtr, address ) )
		return CPubKey();

	CPubKey pubKey;
	if ( !CTrackerNodesManager::getInstance()->getPublicKey( address, pubKey ) )
		return CPubKey();

	return pubKey;
}

void
CRegisterAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
