// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "wallet.h"

#include "common/setResponseVisitor.h"
#include "common/events.h"
#include "common/analyseTransaction.h"
#include "common/authenticationProvider.h"

#include "monitor/passTransactionAction.h"
#include "monitor/filters.h"
#include "monitor/transactionRecordManager.h"
#include "monitor/controller.h"
#include "monitor/provideInfoAction.h"

extern CWallet* pwalletMain;

namespace monitor
{

struct CProcessAsClient;
struct CProcessTransaction;
struct CFetchBalance;
struct CProvideStatusInfo;
struct CAcceptTransaction;
struct CCheckStatus;

unsigned int const LoopTime = 10000;


struct CProcessAsClient : boost::statechart::state< CProcessAsClient, CPassTransactionAction >
{
	CProcessAsClient( my_context ctx ) : my_base( ctx )
	{

		context< CPassTransactionAction >().addRequest(
					new common::CScheduleActionRequest(
						new CProvideInfoAction(
							common::CInfoKind::TrackerInfo
							, common::CMediumKinds::Trackers)
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );

	}

	boost::statechart::result react( common::CTrackerInfoEvent const & _trackerInfoEvent )
	{
		context< CPassTransactionAction >().m_servicingTracker = _trackerInfoEvent.m_trackerInfo;

		return CController::getInstance()->isAdmitted() ? transit< CProcessTransaction >() : transit< CFetchBalance >();

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
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTrackerInfoEvent >,
	boost::statechart::custom_reaction< common::CFailureEvent >
	> reactions;

	CTransaction transaction;
};

struct CFetchBalance : boost::statechart::state< CFetchBalance, CPassTransactionAction >
{
	CFetchBalance( my_context ctx ) : my_base( ctx )
	{
		m_self = common::CAuthenticationProvider::getInstance()->getMyKey().GetID();

		context< CPassTransactionAction >().addRequest(
					new common::CScheduleActionRequest(
						new CProvideInfoAction(
							common::CInfoKind::BalanceAsk
							, context< CPassTransactionAction >().m_servicingTracker.m_key )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );

	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPassTransactionAction >().setResult(
					common::CTransactionAck(
						( int )common::TransactionsStatus::Invalid
						, CTransaction() ) );

		context< CPassTransactionAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return discard_event();
	}


	boost::statechart::result react( common::CAvailableCoinsData const & _availableCoins )
	{
		std::map< uint256, CCoins >::const_iterator iterator = _availableCoins.m_availableCoins.begin();

		CWallet::getInstance()->replaceAvailableCoins( m_self, std::vector< CAvailableCoin >() );

		while( iterator != _availableCoins.m_availableCoins.end() )
		{
			std::vector< CAvailableCoin > availableCoins
					= common::getAvailableCoins(
						iterator->second
						, m_self
						, iterator->first );

			CWallet::getInstance()->addAvailableCoins( m_self, availableCoins );
			iterator++;
		}

		return transit< CProcessTransaction >();
	}

	boost::statechart::result react( common::CFailureEvent const & _failureEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CAvailableCoinsData >,
	boost::statechart::custom_reaction< common::CFailureEvent >
	> reactions;

	CKeyID m_self;
};

struct CProcessTransaction : boost::statechart::state< CProcessTransaction, CPassTransactionAction >
{
	CProcessTransaction( my_context ctx ) : my_base( ctx )
	{
		std::vector< std::pair< CKeyID, int64_t > > outputs;

		outputs.push_back(
					std::pair< CKeyID, int64_t >(
						  context< CPassTransactionAction >().getKeyId()
						, context< CPassTransactionAction >().getAmount() ) );

		CWalletTx tx;
		std::string failReason;

		if (
					!CWallet::getInstance()->CreateTransaction(
						outputs
						, std::vector< CSpendCoins >()
						, context< CPassTransactionAction >().m_servicingTracker.m_key
						, context< CPassTransactionAction >().m_servicingTracker.m_price
						, tx
						, failReason )
				)
		{
			context< CPassTransactionAction >().setResult(
						common::CTransactionAck(
							( int )common::TransactionsStatus::Invalid
							, CTransaction() ) );

			context< CPassTransactionAction >().setExit();
			return;
		}

		context< CPassTransactionAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::ClientTransaction
					, common::CClientTransaction( tx )
					, context< CPassTransactionAction >().getActionKey()
					, new CByKeyMediumFilter( context< CPassTransactionAction >().m_servicingTracker.m_key ) ) );

		context< CPassTransactionAction >().m_hash = tx.GetHash();
		context< CPassTransactionAction >().setResult( common::CTransactionAck( ( int )common::TransactionsStatus::Validated, transaction ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPassTransactionAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CResult result;
		common::convertPayload( orginalMessage, result );

		context< CPassTransactionAction >().addRequest(
					new common::CAckRequest(
						  context< CPassTransactionAction >().getActionKey()
						, orginalMessage.m_header.m_id
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

		if ( result.m_result )
		{
			return transit< CCheckStatus >();
		}
		else
		{
			assert( !"handle problem" );
		}

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

	CTransaction transaction;
};

struct CCheckStatus : boost::statechart::state< CCheckStatus, CPassTransactionAction >
{
	CCheckStatus( my_context ctx ) : my_base( ctx )
	{
		context< CPassTransactionAction >().forgetRequests();
		context< CPassTransactionAction >().addRequest(
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, common::CInfoRequestData( (int)common::CInfoKind::ClientTrasactionStatus, context< CPassTransactionAction >().m_hash )
					, context< CPassTransactionAction >().getActionKey()
					, new CByKeyMediumFilter( context< CPassTransactionAction >().m_servicingTracker.m_key ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPassTransactionAction >().forgetRequests();
		context< CPassTransactionAction >().addRequest(
					new common::CSendMessageRequest(
						common::CPayloadKind::InfoReq
						, common::CInfoRequestData( (int)common::CInfoKind::ClientTrasactionStatus, context< CPassTransactionAction >().m_hash )
						, context< CPassTransactionAction >().getActionKey()
						, new CByKeyMediumFilter( context< CPassTransactionAction >().m_servicingTracker.m_key ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
	if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

	common::CClientTransactionStatus clientTransactionStatus;
	common::convertPayload( orginalMessage, clientTransactionStatus );

		context< CPassTransactionAction >().addRequest(
					new common::CAckRequest(
						  context< CPassTransactionAction >().getActionKey()
						, orginalMessage.m_header.m_id
						, new CByKeyMediumFilter( context< CPassTransactionAction >().m_servicingTracker.m_key ) ) );

		if ( clientTransactionStatus.m_status == common::TransactionsStatus::Validated )
		{
			context< CPassTransactionAction >().setExit();
		}
		else
		{
			context< CPassTransactionAction >().addRequest(
						new common::CTimeEventRequest(
							5000
							, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
		}

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

CPassTransactionAction::CPassTransactionAction( uint256 const & _actionKey )
	: common::CScheduleAbleAction( _actionKey )
{
	initiate();
}


CPassTransactionAction::CPassTransactionAction( CKeyID const & _keyId, int64_t _amount )
	: m_keyId( _keyId )
	, m_amount( _amount )
{
	initiate();
}

void
CPassTransactionAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}
