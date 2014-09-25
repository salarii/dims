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

		context< CPayLocalApplicationAction >().setRequest( new CErrorForAppPaymentProcessing( (dims::CAppError::Enum)indicateErrorEvent->m_error, new CSpecificMediumFilter( context< CPayLocalApplicationAction >().getSocked() ) ) );
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

		CWalletTx tx;
		std::string failReason;

		CClientControl::getInstance()->createTransaction( outputs, std::vector< CAvailableCoin >(), trackerStats, tx, failReason );

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
			context< CPayLocalApplicationAction >().setValidatedTransactionHash( _transactionSendAck.m_transactionSend.GetHash() );
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
						  context< CPayLocalApplicationAction >().getValidatedTransactionHash()
						, new CMediumClassWithExceptionFilter( context< CPayLocalApplicationAction >().getProcessingTrackerPtr(), RequestKind::TransactionStatus, 1 ) ) );
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
							  context< CPayLocalApplicationAction >().getValidatedTransactionHash()
							, new CMediumClassWithExceptionFilter( context< CPayLocalApplicationAction >().getProcessingTrackerPtr(), RequestKind::TransactionStatus, 1 ) ) );
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction<  common::CPending >
	, boost::statechart::custom_reaction< common::CTransactionStatus >
	> reactions;
};


struct CSecondTransaction : boost::statechart::state< CSecondTransaction, CPayLocalApplicationAction >
{
	CSecondTransaction( my_context ctx ) : my_base( ctx )
	{
		std::vector< std::pair< CKeyID, int64_t > > outputs;

		outputs.push_back( std::make_pair( context< CPayLocalApplicationAction >().getTargetKey(), context< CPayLocalApplicationAction >().getValue() ) );

		common::CTrackerStats trackerStats;

		CWalletTx tx;
		std::string failReason;

	//	CAvailableCoin

	//	CClientControl::getInstance()->createTransaction( outputs, , trackerStats, tx, failReason );

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
			context< CPayLocalApplicationAction >().setValidatedTransactionHash( _transactionSendAck.m_transactionSend.GetHash() );
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

struct CSecondCheck : boost::statechart::state< CSecondCheck, CPayLocalApplicationAction >
{
	CSecondCheck( my_context ctx ) : my_base( ctx )
	{
		context< CPayLocalApplicationAction >().setRequest(
					new CTransactionStatusRequest(
						  context< CPayLocalApplicationAction >().getValidatedTransactionHash()
						, new CMediumClassWithExceptionFilter( context< CPayLocalApplicationAction >().getProcessingTrackerPtr(), RequestKind::TransactionStatus, 1 ) ) );
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
			context< CPayLocalApplicationAction >().setRequest( 0 );
		}
		else if ( _transactionStats.m_status == common::TransactionsStatus::Unconfirmed )
		{
			context< CPayLocalApplicationAction >().setRequest(
						new CTransactionStatusRequest(
							  context< CPayLocalApplicationAction >().getValidatedTransactionHash()
							, new CMediumClassWithExceptionFilter( context< CPayLocalApplicationAction >().getProcessingTrackerPtr(), RequestKind::TransactionStatus, 1 ) ) );
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction<  common::CPending >
	, boost::statechart::custom_reaction< common::CTransactionStatus >
	> reactions;
};

CPayLocalApplicationAction::CPayLocalApplicationAction( uintptr_t _socked, CPrivKey const & _privateKey, CKeyID const & _targetKey, int64_t _value,std::vector<CKeyID> const & _trackers, std::vector<CKeyID> const & _monitors )
	: CAction()
	, m_socked( _socked )
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

void
CPayLocalApplicationAction::setValidatedTransactionHash( uint256 _hash )
{
	m_validatedTransactionHash = _hash;
}

uint256
CPayLocalApplicationAction::getValidatedTransactionHash() const
{
	return m_validatedTransactionHash;
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
CPayLocalApplicationAction::getSocked() const
{
	return m_socked;
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


}

