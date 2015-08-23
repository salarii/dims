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
#include "tracker/trackerFilters.h"
#include "tracker/transactionRecordManager.h"
#include "tracker/trackerRequests.h"

extern CWallet* pwalletMain;

namespace tracker
{

struct CProcessAsClient;
struct CValidInNetwork;

unsigned int const LoopTime = 10000;

struct CValidInNetworkEvent : boost::statechart::event< CValidInNetworkEvent >
{
};

struct CInvalidInNetworkEvent : boost::statechart::event< CInvalidInNetworkEvent >
{
};

struct CInitial : boost::statechart::simple_state< CInitial, CPassTransactionAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CValidInNetworkEvent, CValidInNetwork >,
	boost::statechart::transition< CInvalidInNetworkEvent, CProcessAsClient >
	> reactions;
};
extern CWallet* pwalletMain;
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

		CTransactionRecordManager::getInstance()->addClientTransaction( tx );

		context< CPassTransactionAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						  LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	// pool if  given  transaction  executed ????
	// pool transaction status

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
/*		if ( CTransactionRecordManager::getInstance()->getTransaction( _transactionStatus.m_hash, transaction ) )
		{
			context< CPassTransactionAction >().setResult( transaction );
			context< CPassTransactionAction >().setExit();
		}
*/

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;

	CTransaction transaction;
};

struct CProcessAsClient : boost::statechart::state< CProcessAsClient, CPassTransactionAction >
{
	CProcessAsClient( my_context ctx ) : my_base( ctx )
	{
	//	transaction = context< CPassTransactionAction >().getTransaction();

		context< CPassTransactionAction >().addRequest(
					new CTransactionConditionRequest(
						context< CPassTransactionAction >().getActionKey()
						, new CMediumClassFilter( common::CMediumKinds::Trackers, 1 ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPassTransactionAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
//		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), context< CPassTransactionAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		common::CTrackerInfo trackerInfo;
		common::convertPayload( orginalMessage, trackerInfo );

		context< CPassTransactionAction >().addRequest(
					new common::CAckRequest< common::CTrackerTypes >(
						  context< CPassTransactionAction >().getActionKey()
						, orginalMessage.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

		CKeyID keyId;
//		CTrackerNodesManager::getInstance()->getKeyToNode( keyId, _messageResult.m_nodeIndicator );

		std::vector< std::pair< CKeyID, int64_t > > outputs;

		outputs.push_back(
					std::pair< CKeyID, int64_t >(
						  context< CPassTransactionAction >().getKeyId()
						, context< CPassTransactionAction >().getAmount() ) );

		CWalletTx tx;
		std::string failReason;

		common::CTrackerStats tracker;

		tracker.m_price = trackerInfo.m_price; // this  will produce transaction with no tracker output

		if ( pwalletMain->CreateTransaction( outputs, std::vector< CSpendCoins >(), tracker, tx, failReason ) )
		{
			CTransactionRecordManager::getInstance()->addClientTransaction( tx );
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

struct CProcessTransaction : boost::statechart::state< CProcessTransaction, CPassTransactionAction >
{
	CProcessTransaction( my_context ctx ) : my_base( ctx )
	{
//		transaction = context< CPassTransactionAction >().getTransaction();

		context< CPassTransactionAction >().addRequest(
					new CTransactionAsClientRequest(
						transaction
						, context< CPassTransactionAction >().getActionKey()
						, new CMediumClassFilter( common::CMediumKinds::Trackers, 1 ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CPassTransactionAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
//		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), context< CPassTransactionAction >().getPublicKey() ) )
			assert( !"service it somehow" );

		common::CClientTransactionStatus clientTransactionStatus;
		common::convertPayload( orginalMessage, clientTransactionStatus );

		context< CPassTransactionAction >().addRequest(
					new common::CAckRequest< common::CTrackerTypes >(
						  context< CPassTransactionAction >().getActionKey()
						, orginalMessage.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

//		if ( (common::TransactionsStatus::Enum)clientTransactionStatus != common::TransactionsStatus::Confirmed )
		{
			transaction.SetNull();
		}

//		context< CPassTransactionAction >().setResult( transaction );

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

CPassTransactionAction::CPassTransactionAction( CKeyID const & _keyId, int64_t _amount )
	: m_keyId( _keyId )
	, m_amount( _amount )
{
	initiate();
}

void
CPassTransactionAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
