// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "wallet.h"

#include "common/setResponseVisitor.h"

#include "tracker/passTransactionAction.h"
#include "tracker/filters.h"
#include "tracker/transactionRecordManager.h"
#include "tracker/requests.h"
#include "tracker/controller.h"
#include "tracker/getBalanceAction.h"

extern CWallet* pwalletMain;

namespace tracker
{

struct CProcessAsClient;
struct CValidInNetwork;
struct CProcessTransaction;
struct CFetchBalance;
struct CProvideStatusInfo;
struct CAcceptTransaction;
struct CCheckStatus;

unsigned int const LoopTime = 10000;

struct CValidInNetworkEvent : boost::statechart::event< CValidInNetworkEvent >{};
struct CInvalidInNetworkEvent : boost::statechart::event< CInvalidInNetworkEvent >{};
struct CProcessTransactionEvent : boost::statechart::event< CProcessTransactionEvent >{};

// another  way of passing  parameters between states ?? good ?? bad
namespace
{
uintptr_t NodeIndicator;
uint256 Hash;
common::CTrackerStats ServicingTracker;
CTransaction Transaction;

}

struct CPassTransactionInitial : boost::statechart::simple_state< CPassTransactionInitial, CPassTransactionAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CValidInNetworkEvent, CValidInNetwork >,
	boost::statechart::transition< CInvalidInNetworkEvent, CProcessAsClient >,
	boost::statechart::transition< CProcessTransactionEvent, CAcceptTransaction >
	> reactions;
};

struct CAcceptTransaction : boost::statechart::state< CAcceptTransaction, CPassTransactionAction >
{
	CAcceptTransaction( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CClientTransaction clientTransaction;
		common::convertPayload( orginalMessage, clientTransaction );

		context< CPassTransactionAction >().addRequest(
					new common::CAckRequest< common::CTrackerTypes >(
						  context< CPassTransactionAction >().getActionKey()
						, orginalMessage.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

		CTransactionRecordManager::getInstance()->addClientTransaction( clientTransaction.m_transaction );

		common::CSendMessageRequest< common::CTrackerTypes > * request =
				new common::CSendMessageRequest< common::CTrackerTypes >(
					common::CPayloadKind::Result
					, context< CPassTransactionAction >().getActionKey()
					, orginalMessage.m_header.m_id
					, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) );

		request->addPayload( common::CResult( 1 ) );

		context< CPassTransactionAction >().addRequest( request );
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return transit< CProvideStatusInfo >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

};

struct CProvideStatusInfo : boost::statechart::state< CProvideStatusInfo, CPassTransactionAction >
{
	CProvideStatusInfo( my_context ctx ) : my_base( ctx )
	{
		context< CPassTransactionAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						  LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CInfoRequestData infoRequestData;

		common::convertPayload( orginalMessage, infoRequestData );

		context< CPassTransactionAction >().addRequest(
					new common::CAckRequest< common::CTrackerTypes >(
						  context< CPassTransactionAction >().getActionKey()
						, orginalMessage.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

		if ( infoRequestData.m_kind == (int)common::CInfoKind::ClientTrasactionStatus )
		{
			uint256 hash;
			common::castCharVectorToType( infoRequestData.m_payload, &hash );

			int status;
			CTransaction transaction;
			if ( CTransactionRecordManager::getInstance()->getTransaction( hash, transaction ) )
				status = common::TransactionsStatus::Unconfirmed;
			else
				status = common::TransactionsStatus::Validated	;

			common::CSendMessageRequest< common::CTrackerTypes > * request =
					new common::CSendMessageRequest< common::CTrackerTypes >(
						common::CPayloadKind::ClientStatusTransaction
						, context< CPassTransactionAction >().getActionKey()
						, orginalMessage.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) );

			request->addPayload( common::CClientTransactionStatus( status ) );

			context< CPassTransactionAction >().addRequest( request );
		}

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPassTransactionAction >().setExit();
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

struct CValidInNetwork : boost::statechart::state< CValidInNetwork, CPassTransactionAction >
{
	CValidInNetwork( my_context ctx ) : my_base( ctx )
	{
		std::vector< std::pair< CKeyID, int64_t > > outputs;

		outputs.push_back(
					std::pair< CKeyID, int64_t >(
						  context< CPassTransactionAction >().getKeyId()
						, context< CPassTransactionAction >().getAmount() ) );

		CWalletTx tx;
		std::string failReason;

		common::CTrackerStats tracker;
		tracker.m_price = 0; // this  will produce transaction with no tracker output

		if ( !CWallet::getInstance()->CreateTransaction( outputs, std::vector< CSpendCoins >(), tracker, tx, failReason ) )
		{
			context< CPassTransactionAction >().setResult( common::CTransactionAck( ( int )common::TransactionsStatus::Invalid, CTransaction() ) );
			context< CPassTransactionAction >().setExit();
		}

		CTransactionRecordManager::getInstance()->addClientTransaction( tx );
		m_transactionHash = tx.GetHash();

		context< CPassTransactionAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						  LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	// pool if  given  transaction  executed ????
	// pool transaction status

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		CTransaction transaction;
		if ( CTransactionRecordManager::getInstance()->getTransaction( m_transactionHash, transaction ) )
		{
			context< CPassTransactionAction >().setResult( common::CTransactionAck( ( int )common::TransactionsStatus::Confirmed, transaction ) );
			context< CPassTransactionAction >().setExit();
		}

		context< CPassTransactionAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						  LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;

	uint256 m_transactionHash;
};

struct CProcessAsClient : boost::statechart::state< CProcessAsClient, CPassTransactionAction >
{
	CProcessAsClient( my_context ctx ) : my_base( ctx )
	{
		common::CSendMessageRequest< common::CTrackerTypes > * request =
				new common::CSendMessageRequest< common::CTrackerTypes >(
					common::CPayloadKind::InfoReq
					, context< CPassTransactionAction >().getActionKey()
					, new CMediumClassFilter( common::CMediumKinds::Trackers, 1 ) );

		request->addPayload( (int)common::CInfoKind::TrackerInfo, std::vector<unsigned char>() );

		context< CPassTransactionAction >().addRequest( request );
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

		common::CTrackerInfo trackerInfo;
		common::convertPayload( orginalMessage, trackerInfo );

		context< CPassTransactionAction >().addRequest(
					new common::CAckRequest< common::CTrackerTypes >(
						  context< CPassTransactionAction >().getActionKey()
						, orginalMessage.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

		ServicingTracker = common::CTrackerStats( _messageResult.m_pubKey, 0, trackerInfo.m_price );

		return transit< CFetchBalance >();
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

struct CFetchBalance : boost::statechart::state< CFetchBalance, CPassTransactionAction >
{
	CFetchBalance( my_context ctx ) : my_base( ctx )
	{
		context< CPassTransactionAction >().addRequest(
					new common::CScheduleActionRequest< common::CTrackerTypes >(
						new CGetBalanceAction()
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CExecutedIndicator const & _executedIndicator )
	{
		if ( _executedIndicator.m_correct )
			return transit< CProcessTransaction >();

		context< CPassTransactionAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CExecutedIndicator >
	> reactions;
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
						, ServicingTracker
						, tx
						, failReason )
				)
		{
			context< CPassTransactionAction >().setResult(
						common::CTransactionAck(
							( int )common::TransactionsStatus::Invalid
							, CTransaction() ) );

			context< CPassTransactionAction >().setExit();
		}

		common::CSendMessageRequest< common::CTrackerTypes > * request =
				new common::CSendMessageRequest< common::CTrackerTypes >(
					common::CPayloadKind::ClientTransaction
					, context< CPassTransactionAction >().getActionKey()
					, new CMediumClassFilter( common::CMediumKinds::Trackers, 1 ) );

		request->addPayload( common::CClientTransaction( tx ) );

		context< CPassTransactionAction >().addRequest( request );

		Hash = tx.GetHash();
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
					new common::CAckRequest< common::CTrackerTypes >(
						  context< CPassTransactionAction >().getActionKey()
						, orginalMessage.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

		if ( result.m_result )
		{
			NodeIndicator = _messageResult.m_nodeIndicator;
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
		common::CSendMessageRequest< common::CTrackerTypes > * request =
				new common::CSendMessageRequest< common::CTrackerTypes >(
					common::CPayloadKind::InfoReq
					, context< CPassTransactionAction >().getActionKey()
					, new CSpecificMediumFilter( NodeIndicator ) );

		common::CInfoRequestData infoRequestData( (int)common::CInfoKind::ClientTrasactionStatus, std::vector<unsigned char>() );

		common::castTypeToCharVector( &Hash, infoRequestData.m_payload );

		request->addPayload( infoRequestData );

		context< CPassTransactionAction >().addRequest( request );
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

	common::CClientTransactionStatus clientTransactionStatus;
	common::convertPayload( orginalMessage, clientTransactionStatus );

		context< CPassTransactionAction >().addRequest(
					new common::CAckRequest< common::CTrackerTypes >(
						  context< CPassTransactionAction >().getActionKey()
						, orginalMessage.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

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
	: common::CScheduleAbleAction< common::CTrackerTypes >( _actionKey )
{
	initiate();
	process_event( CProcessTransactionEvent() );
}


CPassTransactionAction::CPassTransactionAction( CKeyID const & _keyId, int64_t _amount )
	: m_keyId( _keyId )
	, m_amount( _amount )
{
	initiate();
	if ( CController::getInstance()->isConnected() )
		process_event( CValidInNetworkEvent() );
	else
		process_event( CInvalidInNetworkEvent() );
}

void
CPassTransactionAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
