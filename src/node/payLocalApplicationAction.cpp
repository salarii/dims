// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/transition.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "payLocalApplicationAction.h"
#include "common/nodeMessages.h"
#include "common/setResponseVisitor.h"
#include "common/medium.h"
#include "common/analyseTransaction.h"

#include "clientFilters.h"
#include "clientRequests.h"
#include "clientEvents.h"
#include "clientControl.h"

#include "configureNodeActionHadler.h"
#include "serialize.h"
#include "base58.h"

using namespace common;

namespace client
{

struct CServiceByTrackerEvent : boost::statechart::event< CServiceByTrackerEvent >
{
	CServiceByTrackerEvent( CKeyID const & _keyId ):m_keyId( _keyId ) {}
	CKeyID const m_keyId;
};

struct CResolveByMonitorEvent : boost::statechart::event< CResolveByMonitorEvent >
{
};

struct CIndicateErrorEvent : boost::statechart::event< CIndicateErrorEvent >
{
	CIndicateErrorEvent( int const _error ):m_error( _error ){}
	int const m_error;
};

struct CIndicateErrorCondition : boost::statechart::state< CIndicateErrorCondition, CPayLocalApplicationAction >
{
	CIndicateErrorCondition( my_context ctx ) : my_base( ctx )
	{
		CIndicateErrorEvent const* indicateErrorEvent = dynamic_cast< CIndicateErrorEvent const* >( simple_state::triggering_event() );

		context< CPayLocalApplicationAction >().setRequest( new CErrorForAppPaymentProcessing( (dims::CAppError::Enum)indicateErrorEvent->m_error, new CSpecificMediumFilter( context< CPayLocalApplicationAction >().getSocket() ) ) );
	}

	boost::statechart::result react( common::CPending const & _pending )
	{
		context< CPayLocalApplicationAction >().setRequest( 0 );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CPending >
	> reactions;
};

struct CResolveByMonitor : boost::statechart::state< CResolveByMonitor, CPayLocalApplicationAction >
{
	CResolveByMonitor( my_context ctx ) : my_base( ctx )
	{}
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

		CClientControl::getInstance()->createTransaction( outputs, std::vector< CAvailableCoin >(), trackerStats, tx, failReason );

		context< CPayLocalApplicationAction >().setServicingTracker( trackerStats.m_key );

		context< CPayLocalApplicationAction >().setRequest( new CTransactionSendRequest( tx, new CMediumByKeyFilter( serviceByTrackerEvent->m_keyId ) ) );
	}

	boost::statechart::result react( common::CPending const & _pending )
	{
		context< CPayLocalApplicationAction >().setProcessingTrackerPtr( _pending.m_networkPtr );
		context< CPayLocalApplicationAction >().setRequest( new CInfoRequestContinue( _pending.m_token, new CSpecificMediumFilter( _pending.m_networkPtr ) ) );
		return discard_event();
	}

	boost::statechart::result react( CTransactionAckEvent const & _transactionSendAck )
	{
// todo, check status and validity of the transaction propagated
		if ( _transactionSendAck.m_status == common::TransactionsStatus::Validated )
		{
			CClientControl::getInstance()->addTransactionToModel( _transactionSendAck.m_transactionSend );
			context< CPayLocalApplicationAction >().setFirstTransaction( _transactionSendAck.m_transactionSend );
			return transit< CCheckTransactionStatus >();
		}
		else
		{
			context< CPayLocalApplicationAction >().setRequest( 0 );
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction< common::CPending >
	, boost::statechart::custom_reaction< CTransactionAckEvent >
	> reactions;

};

struct CCheckAppData : boost::statechart::state< CCheckAppData, CPayLocalApplicationAction >
{
	CCheckAppData( my_context ctx ) : my_base( ctx )
	{
	}

	typedef boost::mpl::list<
	  boost::statechart::transition< CIndicateErrorEvent, CIndicateErrorCondition >
	, boost::statechart::transition< CServiceByTrackerEvent, CServiceByTracker >
	, boost::statechart::transition< CResolveByMonitorEvent, CResolveByMonitor >
	> reactions;
};

struct CSecondTransaction;

struct CCheckTransactionStatus : boost::statechart::state< CCheckTransactionStatus, CPayLocalApplicationAction >
{
	CCheckTransactionStatus( my_context ctx ) : my_base( ctx )
	{
		context< CPayLocalApplicationAction >().setRequest(
					new CTransactionStatusRequest(
						  context< CPayLocalApplicationAction >().getFirstTransaction().GetHash()
						, new CSpecificMediumFilter( context< CPayLocalApplicationAction >().getProcessingTrackerPtr() ) ) );
	}

	boost::statechart::result react( common::CPending const & _pending )
	{
		context< CPayLocalApplicationAction >().setRequest( new CInfoRequestContinue( _pending.m_token, new CSpecificMediumFilter( _pending.m_networkPtr ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CTransactionStatus const & _transactionStats )
	{
		if ( _transactionStats.m_status == common::TransactionsStatus::Confirmed )
		{
			return  transit< CSecondTransaction >();
		}
		else if ( _transactionStats.m_status == common::TransactionsStatus::Unconfirmed )
		{
			context< CPayLocalApplicationAction >().setRequest(
						new CTransactionStatusRequest(
							  context< CPayLocalApplicationAction >().getFirstTransaction().GetHash()
							, new CSpecificMediumFilter( context< CPayLocalApplicationAction >().getProcessingTrackerPtr() ) ) );
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction<  common::CPending >
	, boost::statechart::custom_reaction< common::CTransactionStatus >
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

		CTxOut txOut;
		unsigned int id;
		if ( !common::findOutputInTransaction( firstTransaction, context< CPayLocalApplicationAction >().getPrivKey().GetPubKey().GetID(), txOut, id ) )
			assert( !"something went wrong" );

		std::vector< CAvailableCoin > coinsToUse;
		coinsToUse.push_back( CAvailableCoin( txOut, id, firstTransaction.GetHash() ) );

		CClientControl::getInstance()->addKey( context< CPayLocalApplicationAction >().getPrivKey(), context< CPayLocalApplicationAction >().getPrivKey().GetPubKey(), true );

		CClientControl::getInstance()->createTransaction( outputs, coinsToUse, context< CPayLocalApplicationAction >().getTrackerStats(), tx, failReason );

		CClientControl::getInstance()->removeKey( context< CPayLocalApplicationAction >().getPrivKey().GetPubKey() );

		context< CPayLocalApplicationAction >().setRequest( new CTransactionSendRequest( tx, new CSpecificMediumFilter( context< CPayLocalApplicationAction >().getProcessingTrackerPtr() ) ) );
	}

	boost::statechart::result react( common::CPending const & _pending )
	{
		context< CPayLocalApplicationAction >().setProcessingTrackerPtr( _pending.m_networkPtr );
		context< CPayLocalApplicationAction >().setRequest( new CInfoRequestContinue( _pending.m_token, new CSpecificMediumFilter( _pending.m_networkPtr ) ) );
		return discard_event();
	}

	boost::statechart::result react( CTransactionAckEvent const & _transactionSendAck )
	{
// todo, check status and validity of the transaction propagated
		if ( _transactionSendAck.m_status == common::TransactionsStatus::Validated )
		{
			CClientControl::getInstance()->addTransactionToModel( _transactionSendAck.m_transactionSend );
			context< CPayLocalApplicationAction >().setSecondTransaction( _transactionSendAck.m_transactionSend );
			return transit< CSecondCheck >();
		}
		else
		{
			context< CPayLocalApplicationAction >().setRequest( 0 );
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction< common::CPending >
	, boost::statechart::custom_reaction< CTransactionAckEvent >
	> reactions;

};

struct CSendTransactionData;

struct CSecondCheck : boost::statechart::state< CSecondCheck, CPayLocalApplicationAction >
{
	CSecondCheck( my_context ctx ) : my_base( ctx )
	{
		context< CPayLocalApplicationAction >().setRequest(
					new CTransactionStatusRequest(
						  context< CPayLocalApplicationAction >().getSecondTransaction().GetHash()
						, new CSpecificMediumFilter( context< CPayLocalApplicationAction >().getProcessingTrackerPtr() ) ) );
	}

	boost::statechart::result react( common::CPending const & _pending )
	{
		context< CPayLocalApplicationAction >().setRequest( new CInfoRequestContinue( _pending.m_token, new CSpecificMediumFilter( _pending.m_networkPtr ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CTransactionStatus const & _transactionStats )
	{
		if ( _transactionStats.m_status == common::TransactionsStatus::Confirmed )
		{
			return transit< CSendTransactionData >();
		}
		else if ( _transactionStats.m_status == common::TransactionsStatus::Unconfirmed )
		{
			context< CPayLocalApplicationAction >().setRequest(
						new CTransactionStatusRequest(
							  context< CPayLocalApplicationAction >().getSecondTransaction().GetHash()
							, new CMediumClassWithExceptionFilter( context< CPayLocalApplicationAction >().getProcessingTrackerPtr(), RequestKind::TransactionStatus, 1 ) ) );
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction<  common::CPending >
	, boost::statechart::custom_reaction< common::CTransactionStatus >
	> reactions;
};

struct CSendTransactionData : boost::statechart::state< CSendTransactionData, CPayLocalApplicationAction >
{
	CSendTransactionData( my_context ctx ) : my_base( ctx )
	{
		common::CTransactionStatus const* transactionStatus = dynamic_cast< CTransactionStatus const* >( simple_state::triggering_event() );

		context< CPayLocalApplicationAction >().setRequest(
					new CProofTransactionAndStatusRequest(
						  context< CPayLocalApplicationAction >().getSecondTransaction()
						, transactionStatus->m_signature
						, context< CPayLocalApplicationAction >().getServicingTracker()
						, new CSpecificMediumFilter( context< CPayLocalApplicationAction >().getSocket() ) ) );
	}

	boost::statechart::result react( common::CPending const & _pending )
	{
		context< CPayLocalApplicationAction >().setRequest( 0 );
		return discard_event();
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

	std::vector<CKeyID>::const_iterator iterator = m_trackers.begin();

	CTrackerLocalRanking::getInstance()->isValidTrackerKnown( CKeyID() );

	if ( !CClientControl::getInstance()->executePaymentMessageBox() )
	{
		process_event( CIndicateErrorEvent( dims::CAppError::RefusedByClient ) );
		return;
	}

	while( iterator != m_trackers.end() )
	{
		if ( CTrackerLocalRanking::getInstance()->isValidTrackerKnown( *iterator ) )
		{
			process_event( CServiceByTrackerEvent( *iterator ) );
			return;
		}
	}

	iterator = m_monitors.begin();

	while( iterator != m_monitors.end() )
	{
		if ( CTrackerLocalRanking::getInstance()->isValidMonitorKnown( *iterator ) )
		{
			process_event( CResolveByMonitorEvent() );
			return;
		}
	}
}

void
CPayLocalApplicationAction::accept( common::CSetResponseVisitor< NodeResponses > & _visitor )
{
	_visitor.visit( *this );
}


CRequest< NodeResponses > *
CPayLocalApplicationAction::execute()
{
	return m_request;
}

void
CPayLocalApplicationAction::setRequest( common::CRequest< NodeResponses > * _request )
{
	m_request = _request;
}

void
CPayLocalApplicationAction::setProcessingTrackerPtr( uintptr_t _ptr )
{
	m_processingTrackerPtr = _ptr;
}

uintptr_t
CPayLocalApplicationAction::getProcessingTrackerPtr() const
{
	return m_processingTrackerPtr;
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

}

