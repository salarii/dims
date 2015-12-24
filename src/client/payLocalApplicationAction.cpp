// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/transition.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/nodeMessages.h"
#include "common/setResponseVisitor.h"
#include "common/medium.h"
#include "common/analyseTransaction.h"
#include "common/requests.h"

#include "client/payLocalApplicationAction.h"
#include "client/control.h"

#include "filters.h"
#include "requests.h"
#include "events.h"
#include "control.h"
#include "sendInfoRequestAction.h"
#include "serialize.h"
#include "base58.h"

using namespace common;

namespace client
{

const unsigned MonitorAskLoopTime = 20;//seconds

struct CIndicateErrorCondition;
struct CServiceByTracker;
struct CResolveByMonitor;

struct CServiceByTrackerEvent : boost::statechart::event< CServiceByTrackerEvent >
{
	CServiceByTrackerEvent( CKeyID const & _keyId ):m_keyId( _keyId ) {}
	CKeyID const m_keyId;
};

struct CResolveByMonitorEvent : boost::statechart::event< CResolveByMonitorEvent >
{
	CResolveByMonitorEvent( CKeyID const & _keyId ):m_keyId( _keyId ) {}
	CKeyID const m_keyId;
};

struct CIndicateErrorEvent : boost::statechart::event< CIndicateErrorEvent >
{
	CIndicateErrorEvent( dims::CAppError::Enum const _error ):m_error( _error ){}
	dims::CAppError::Enum const m_error;
};

struct CCheckAppData : boost::statechart::state< CCheckAppData, CPayLocalApplicationAction >
{
	CCheckAppData( my_context ctx ) : my_base( ctx )
	{
		context< CPayLocalApplicationAction >().addRequest(
					new common::CTimeEventRequest( 1000, new CMediumClassFilter( ClientMediums::Time ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		if ( CClientControl::getInstance()->isClientReady() && context< CPayLocalApplicationAction >().m_userEnabled )
		{
			std::vector<CKeyID>::const_iterator iterator = context< CPayLocalApplicationAction >().getTrackers().begin();

			while( iterator != context< CPayLocalApplicationAction >().getTrackers().end() )
			{
				if ( CTrackerLocalRanking::getInstance()->isValidTrackerKnown( *iterator ) )
				{
					context< CPayLocalApplicationAction >().process_event( CServiceByTrackerEvent( *iterator ) );
					return discard_event();
				}
				iterator++;
			}

			iterator = context< CPayLocalApplicationAction >().getMonitors().begin();

			while( iterator != context< CPayLocalApplicationAction >().getMonitors().end() )
			{
				if ( CTrackerLocalRanking::getInstance()->isValidMonitorKnown( *iterator ) )
				{
					context< CPayLocalApplicationAction >().process_event( CResolveByMonitorEvent( *iterator ) );
					return discard_event();
				}

				iterator++;
			}
		}
		else
		{
			context< CPayLocalApplicationAction >().addRequest(
						new common::CTimeEventRequest( 1000, new CMediumClassFilter( ClientMediums::Time ) ) );
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction< common::CTimeEvent >
	, boost::statechart::transition< CIndicateErrorEvent, CIndicateErrorCondition >
	, boost::statechart::transition< CServiceByTrackerEvent, CServiceByTracker >
	, boost::statechart::transition< CResolveByMonitorEvent, CResolveByMonitor >
	> reactions;
};

struct CIndicateErrorCondition : boost::statechart::state< CIndicateErrorCondition, CPayLocalApplicationAction >
{
	CIndicateErrorCondition( my_context ctx ) : my_base( ctx )
	{
		CIndicateErrorEvent const* indicateErrorEvent = dynamic_cast< CIndicateErrorEvent const* >( simple_state::triggering_event() );

		context< CPayLocalApplicationAction >().forgetRequests();
		context< CPayLocalApplicationAction >().addRequest( new CErrorForAppPaymentProcessing( indicateErrorEvent->m_error, new CSpecificMediumFilter( context< CPayLocalApplicationAction >().getSocket() ) ) );
		context< CPayLocalApplicationAction >().setExit();
	}
};

struct CServiceByTracker;

struct CResolveByMonitor : boost::statechart::state< CResolveByMonitor, CPayLocalApplicationAction >
{
	CResolveByMonitor( my_context ctx ) : my_base( ctx )
	{
		CResolveByMonitorEvent const* serviceByMonitorEvent = dynamic_cast< CResolveByMonitorEvent const* >( simple_state::triggering_event() );

		context< CPayLocalApplicationAction >().forgetRequests();

		common::CSendClientMessageRequest * request =
				new common::CSendClientMessageRequest(
					common::CMainRequestType::MonitorInfoReq
					, new CMediumByKeyFilter( serviceByMonitorEvent->m_keyId ) );

		context< CPayLocalApplicationAction >().addRequest( request );
	}

	boost::statechart::result react( common::CNoMedium const & _noMedium )
	{
		assert( !"given monitor does not exist" );
		return discard_event();
	}

	boost::statechart::result react( common::CClientMessageResponse const & _message )
	{
		common::CMonitorData monitorData;
		convertClientPayload( _message.m_clientMessage, monitorData );

		context< CPayLocalApplicationAction >().setMonitorData( monitorData );

		common::CTrackerStats trackerStats, best;

		CPubKey key;
		CTrackerLocalRanking::getInstance()->getNodeKey( _message.m_ip, key );

		context< CPayLocalApplicationAction >().setServicingMonitor( key );

		key.Verify( common::hashMonitorData( monitorData ), monitorData.m_signed );
		unsigned int bestFee = -1;
		BOOST_FOREACH( common::CTrackerData const & tracker, monitorData.m_trackers )
		{
			if ( CTrackerLocalRanking::getInstance()->getTrackerStats( tracker.m_publicKey.GetID(), trackerStats ) )
			{
				if ( bestFee > trackerStats.m_price )
					best = trackerStats;
			}
		}

		context< CPayLocalApplicationAction >().process_event( CServiceByTrackerEvent( best.m_key.GetID() ) );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CNoMedium >,
	boost::statechart::transition< CServiceByTrackerEvent, CServiceByTracker >,
	boost::statechart::custom_reaction< common::CClientMessageResponse >
	> reactions;
};

struct CCheckTransactionStatus;

struct CServiceByTracker : boost::statechart::state< CServiceByTracker, CPayLocalApplicationAction >
{
	CServiceByTracker( my_context ctx ) : my_base( ctx )
	{
		CServiceByTrackerEvent const* serviceByTrackerEvent = dynamic_cast< CServiceByTrackerEvent const* >( simple_state::triggering_event() );

		std::vector< std::pair< CKeyID, int64_t > > outputs;

		outputs.push_back( std::make_pair( context< CPayLocalApplicationAction >().getPrivKey().GetPubKey().GetID(), context< CPayLocalApplicationAction >().getValue() ) );

		common::CTrackerStats trackerStats;

		bool trackerDataPresent = CTrackerLocalRanking::getInstance()->getTrackerStats( serviceByTrackerEvent->m_keyId, trackerStats );

		assert( trackerDataPresent );

		context< CPayLocalApplicationAction >().setTrackerStats( trackerStats );

		CWalletTx tx;
		std::string failReason;

		if ( !CWallet::getInstance()->CreateTransaction( outputs, std::vector< CSpendCoins >(), trackerStats.m_key, trackerStats.m_price,tx, failReason ) )
		{
			context< CPayLocalApplicationAction >().addRequest( new CErrorForAppPaymentProcessing( dims::CAppError::NotEnoughFunds, new CSpecificMediumFilter( context< CPayLocalApplicationAction >().getSocket() ) ) );
			context< CPayLocalApplicationAction >().setExit();

			return;
		}
		context< CPayLocalApplicationAction >().setServicingTracker( trackerStats.m_key );

		context< CPayLocalApplicationAction >().setFirstInitialHash( tx.GetHash() );

		context< CPayLocalApplicationAction >().forgetRequests();

		context< CPayLocalApplicationAction >().addRequest(
				new common::CSendClientMessageRequest(
					common::CMainRequestType::Transaction
					, tx
					, tx.GetHash()
					, new CMediumByKeyFilter( serviceByTrackerEvent->m_keyId ) ) );
	}

	boost::statechart::result react( common::CClientMessageResponse const & _message )
	{
		common::CTransactionAck transactionAckData;
		convertClientPayload( _message.m_clientMessage, transactionAckData );

		if ( transactionAckData.m_status == (int)common::TransactionsStatus::Validated )
		{
			CClientControl::getInstance()->addTransactionToModel( transactionAckData.m_transaction );
			context< CPayLocalApplicationAction >().setFirstTransaction( transactionAckData.m_transaction );
			return transit< CCheckTransactionStatus >();
		}
		else
		{
			context< CPayLocalApplicationAction >().forgetRequests();
		}
		return discard_event();

	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CClientMessageResponse >
	> reactions;

};

struct CSecondTransaction;

struct CCheckTransactionStatus : boost::statechart::state< CCheckTransactionStatus, CPayLocalApplicationAction >
{
	CCheckTransactionStatus( my_context ctx ) : my_base( ctx )
	{
		context< CPayLocalApplicationAction >().forgetRequests();

		context< CPayLocalApplicationAction >().addRequest(
				new common::CSendClientMessageRequest(
					common::CMainRequestType::TransactionStatusReq
					, common::CClientTransactionStatusAsk(context< CPayLocalApplicationAction >().getFirstTransaction().GetHash())
					, new CMediumByKeyFilter( context< CPayLocalApplicationAction >().getTrackerStats().m_key.GetID() ) ) );
	}

	boost::statechart::result react( common::CClientMessageResponse const & _message )
	{
		common::CTransactionStatus transactionStatus;
		convertClientPayload( _message.m_clientMessage, transactionStatus );

		if ( transactionStatus.m_status == (int)common::TransactionsStatus::Confirmed )
		{
			CClientControl::getInstance()->transactionAddmited( context< CPayLocalApplicationAction >().getFirstInitailHash(), context< CPayLocalApplicationAction >().getFirstTransaction() );

			return  transit< CSecondTransaction >();
		}
		else if ( transactionStatus.m_status == (int)common::TransactionsStatus::Unconfirmed )
		{
			context< CPayLocalApplicationAction >().forgetRequests();

			context< CPayLocalApplicationAction >().addRequest(
					new common::CSendClientMessageRequest(
						common::CMainRequestType::TransactionStatusReq
						, common::CClientTransactionStatusAsk(context< CPayLocalApplicationAction >().getFirstTransaction().GetHash() )
						, new CMediumClassFilter( ClientMediums::TrackersRep, 1 ) ) );
		}
		return discard_event();

	}
	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CClientMessageResponse >
	> reactions;
};

struct CSecondCheck;

struct CSecondTransaction : boost::statechart::state< CSecondTransaction, CPayLocalApplicationAction >
{
	CSecondTransaction( my_context ctx ) : my_base( ctx )
	{
		std::vector< std::pair< CKeyID, int64_t > > outputs;

		outputs.push_back( std::make_pair( context< CPayLocalApplicationAction >().getTargetKey(), context< CPayLocalApplicationAction >().getValue() ) );

		CWalletTx tx;

		std::string failReason;

		CTransaction const & firstTransaction = context< CPayLocalApplicationAction >().getFirstTransaction();

		std::vector< CTxOut > txOuts;
		std::vector< unsigned int > ids;
		if ( !common::findOutputInTransaction( firstTransaction, context< CPayLocalApplicationAction >().getPrivKey().GetPubKey().GetID(), txOuts, ids ) )
			assert( !"something went wrong" );

		assert( txOuts.size() == ids.size() );

		std::vector< CSpendCoins > coinsToUse;
		for ( unsigned int i = 0; i < txOuts.size(); ++i)
		{
			coinsToUse.push_back( CSpendCoins( txOuts[i], ids[i], firstTransaction.GetHash(), context< CPayLocalApplicationAction >().getPrivKey() ) );
		}
		if ( !CWallet::getInstance()->CreateTransaction( outputs, coinsToUse, context< CPayLocalApplicationAction >().getTrackerStats().m_key,context< CPayLocalApplicationAction >().getTrackerStats().m_price, tx, failReason ) )
		{
			context< CPayLocalApplicationAction >().addRequest( new CErrorForAppPaymentProcessing( dims::CAppError::NotEnoughFunds, new CSpecificMediumFilter( context< CPayLocalApplicationAction >().getSocket() ) ) );
			context< CPayLocalApplicationAction >().setExit();

			return;
		}
		context< CPayLocalApplicationAction >().forgetRequests();

		context< CPayLocalApplicationAction >().addRequest(
				new common::CSendClientMessageRequest(
					common::CMainRequestType::Transaction
					, common::CClientTransactionSend(tx)
					, tx.GetHash()
					, new CMediumByKeyFilter( context< CPayLocalApplicationAction >().getTrackerStats().m_key.GetID() ) ) );
	}

	boost::statechart::result react( common::CClientMessageResponse const & _message )
	{
		common::CTransactionAck transactionAckData;
		convertClientPayload( _message.m_clientMessage, transactionAckData );

		if ( transactionAckData.m_status == (int)common::TransactionsStatus::Validated )
		{
			CClientControl::getInstance()->addTransactionToModel( transactionAckData.m_transaction );
			context< CPayLocalApplicationAction >().setSecondTransaction( transactionAckData.m_transaction );
			return transit< CSecondCheck >();
		}
		else
		{
			context< CPayLocalApplicationAction >().forgetRequests();
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CClientMessageResponse >
	> reactions;

};

struct CSendTransactionData;

struct CSecondCheck : boost::statechart::state< CSecondCheck, CPayLocalApplicationAction >
{
	CSecondCheck( my_context ctx ) : my_base( ctx )
	{
		context< CPayLocalApplicationAction >().forgetRequests();

		context< CPayLocalApplicationAction >().addRequest(
				new common::CSendClientMessageRequest(
					common::CMainRequestType::TransactionStatusReq
					, common::CClientTransactionStatusAsk(context< CPayLocalApplicationAction >().getSecondTransaction().GetHash())
					, new CMediumByKeyFilter( context< CPayLocalApplicationAction >().getTrackerStats().m_key.GetID() ) ) );

	}

	boost::statechart::result react( common::CClientMessageResponse const & _message )
	{
		common::CTransactionStatus transactionStatus;
		convertClientPayload( _message.m_clientMessage, transactionStatus );

		if ( transactionStatus.m_status == (int)common::TransactionsStatus::Confirmed )
		{
			context< CPayLocalApplicationAction >().m_signature = transactionStatus.m_signature;
			return transit< CSendTransactionData >();
		}
		else if ( transactionStatus.m_status == (int)common::TransactionsStatus::Unconfirmed )
		{
			context< CPayLocalApplicationAction >().forgetRequests();

			context< CPayLocalApplicationAction >().addRequest(
					new common::CSendClientMessageRequest(
						common::CMainRequestType::TransactionStatusReq
						, common::CClientTransactionStatusAsk(context< CPayLocalApplicationAction >().getSecondTransaction().GetHash())
						, new CMediumClassWithExceptionFilter( _message.m_nodePtr, ClientMediums::TrackersRep, 1 ) ) );

		}
		return discard_event();
	}

	typedef boost::mpl::list<
		boost::statechart::custom_reaction< common::CClientMessageResponse >
	> reactions;
};

struct CSendTransactionData : boost::statechart::state< CSendTransactionData, CPayLocalApplicationAction >
{
	CSendTransactionData( my_context ctx ) : my_base( ctx )
	{
		context< CPayLocalApplicationAction >().forgetRequests();

		context< CPayLocalApplicationAction >().addRequest(
					new CProofTransactionAndStatusRequest(
						  context< CPayLocalApplicationAction >().getSecondTransaction()
						, context< CPayLocalApplicationAction >().m_signature
						, context< CPayLocalApplicationAction >().getServicingTracker()
						, context< CPayLocalApplicationAction >().getMonitorData()
						, context< CPayLocalApplicationAction >().getServicingMonitor()
						, new CSpecificMediumFilter( context< CPayLocalApplicationAction >().getSocket() ) ) );
	}
};

CPayLocalApplicationAction::CPayLocalApplicationAction( uintptr_t _socket, CPrivKey const & _privateKey, CKeyID const & _targetKey, int64_t _value,std::vector<CKeyID> const & _trackers, std::vector<CKeyID> const & _monitors )
	: CAction()
	, m_socket( _socket )
	, m_privateKey(_privateKey)
	, m_value( _value )
	, m_targetKey( _targetKey )
	, m_trackers( _trackers )
	, m_monitors( _monitors )
{
	initiate();

	if ( !CClientControl::getInstance()->executePaymentMessageBox(context< CPayLocalApplicationAction >().getValue()) )
	{
		context< CPayLocalApplicationAction >().process_event( CIndicateErrorEvent( dims::CAppError::RefusedByClient ) );
	}
	m_userEnabled = true;
}

void
CPayLocalApplicationAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

CPrivKey
CPayLocalApplicationAction::getPrivAppKey() const
{
	return m_privateKey;
}

std::vector<CKeyID> const &
CPayLocalApplicationAction::getTrackers() const
{
	return m_trackers;
}

std::vector<CKeyID> const &
CPayLocalApplicationAction::getMonitors() const
{
	return m_monitors;
}

CKey
CPayLocalApplicationAction::getPrivKey() const
{
	CKey key;
	key.SetPrivKey( m_privateKey ,false );
	return key;
}

CKeyID
CPayLocalApplicationAction::getTargetKey() const
{
	return m_targetKey;
}

int64_t
CPayLocalApplicationAction::getValue() const
{
	return m_value;
}

uintptr_t
CPayLocalApplicationAction::getSocket() const
{
	return m_socket;
}

void
CPayLocalApplicationAction::setFirstTransaction( CTransaction const & _firstTransaction )
{
	m_firstTransaction = _firstTransaction;
}

CTransaction const &
CPayLocalApplicationAction::getFirstTransaction() const
{
	return m_firstTransaction;
}

void
CPayLocalApplicationAction::setSecondTransaction( CTransaction const & _secondTransaction )
{
	m_secondTransaction = _secondTransaction;
}

CTransaction const &
CPayLocalApplicationAction::getSecondTransaction() const
{
	return m_secondTransaction;
}

void
CPayLocalApplicationAction::setTrackerStats( common::CTrackerStats const & _trackerStats )
{
	m_trackerStats = _trackerStats;
}

common::CTrackerStats
CPayLocalApplicationAction::getTrackerStats() const
{
	return m_trackerStats;
}

void
CPayLocalApplicationAction::setServicingTracker( CPubKey const & _servicingTracker )
{
	m_servicingTracker = _servicingTracker;
}

CPubKey const &
CPayLocalApplicationAction::getServicingTracker() const
{
	return m_servicingTracker;
}

CPayLocalApplicationAction::~CPayLocalApplicationAction()
{
	CClientControl::getInstance()->updateTotalBalance( CWallet::getInstance()->GetBalance() );
}

}

