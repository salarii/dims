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
#include "tracker/trackerRequests.h"
#include "tracker/selfWallet.h"
#include "tracker/synchronizationAction.h"
#include "tracker/getBalanceAction.h"
#include "tracker/trackerController.h"

namespace tracker
{

/*
get information about registration status
get  conditions of registration

------>> 1 <<-------
-no other trackes
-start operating
-prepare transaction emit it
-send hash to monitor
- conclude
------>> 2 <<-------
- other trackers
- ask for balance ( send  proof of registration if refused )
- prepare  transaction and emit
- send hash to monitor
- conclude
*/



//milisec
unsigned int const WaitTime = 20000;
unsigned int const MoneyWaitTime = 30000;

struct CFreeRegistration;
struct CSynchronize;
struct CNoTrackers;


struct CInitiateRegistration : boost::statechart::state< CInitiateRegistration, CRegisterAction >
{
	CInitiateRegistration( my_context ctx )
		: my_base( ctx )
	{
		LogPrintf("register action: %p initiate registration \n", &context< CRegisterAction >() );
		context< CRegisterAction >().dropRequests();
		context< CRegisterAction >().addRequest(
		 new common::CTimeEventRequest< common::CTrackerTypes >( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

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
		context< CRegisterAction >().dropRequests();
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

		context< CRegisterAction >().dropRequests();

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
		context< CRegisterAction >().dropRequests();
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

/*
- montor  allowes  him to work
- send synchronization request
- start  synchronization  action
- after conclusion try to create transaction

*/
struct CSynchronize : boost::statechart::state< CSynchronize, CRegisterAction >
{
	CSynchronize( my_context ctx )
		: my_base( ctx )
	{
		context< CRegisterAction >().dropRequests();
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

		CTrackerController::getInstance()->setConnected( true );

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

		//	CTransactionRecordManager::getInstance()->addClientTransaction( _transactionMessage.m_transaction );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{/*
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
	*/	return discard_event();
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

		/*		context< CRegisterAction >().addRequest(
							new CRegisterProofRequest(
						_transactionAckEvent.m_transactionSend.GetHash()
						, context< CRegisterAction >().getActionKey()
						, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) ) );*/
	//	_transactionAckEvent.m_transactionSend
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

		context< CRegisterAction >().dropRequests();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
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
		context< CRegisterAction >().dropRequests();
		context< CRegisterAction >().addRequest(
					new common::CScheduleActionRequest< common::CTrackerTypes >(
						new CGetBalanceAction()
						, new CMediumClassFilter( common::CMediumKinds::Schedule ) ) );
	}

	boost::statechart::result react( common::CExecutedIndicator const & _executedIndicator )
	{
		context< CRegisterAction >().dropRequests();

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

CRegisterAction::CRegisterAction( uintptr_t _nodePtr )
	: m_nodePtr( _nodePtr )
{
	initiate();
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
