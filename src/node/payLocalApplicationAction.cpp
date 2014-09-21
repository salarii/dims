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

struct CResolveByMonitor : boost::statechart::state< CPrepareAndSendTransaction, CResolveByMonitor >
{
};

struct CServiceByTracker : boost::statechart::state< CPrepareAndSendTransaction, CServiceByTracker >
{
	CServiceByTracker( my_context ctx ) : my_base( ctx )
	{
		CServiceByTrackerEvent const* serviceByTrackerEvent = dynamic_cast< CServiceByTrackerEvent const* >( simple_state::triggering_event() );
		serviceByTrackerEvent
	}

};

struct CCheckAppData : boost::statechart::state< CPrepareAndSendTransaction, CCheckAppData >
{
	CCheckAppData( my_context ctx ) : my_base( ctx )
	{
	}

};

struct CPrepareAndSendTransaction : boost::statechart::state< CPrepareAndSendTransaction, CPrepareAndSendTransaction >
{
	CPrepareAndSendTransaction( my_context ctx ) : my_base( ctx )
	{
		context< CPayLocalApplicationAction >().setRequest( new CTransactionSendRequest( context< CPayLocalApplicationAction >().getTransaction(), new CMediumClassFilter( RequestKind::Transaction, 1 ) ) );
	}
//  ack here
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

CPayLocalApplicationAction::CPayLocalApplicationAction( CPrivKey const & _privateKey, std::vector<CKeyID> const & _trackers, std::vector<CKeyID> const & _monitors )
	: CAction()
	, m_privateKey(_privateKey)
	, m_trackers( _trackers )
	, m_monitors( _monitors )
{
	initiate();

	std::vector<CKeyID>::const_iterator iterator = m_trackers.begin();

	while( iterator != m_trackers.end() )
	{
		process_event(  );
	}

	iterator = m_monitors.begin();

	while( iterator != m_monitors.end() )
	{
		process_event(  );
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

CTransaction const &
CPayLocalApplicationAction::getTransaction() const
{
	return m_transaction;
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

CTransactionStatusRequest::CTransactionStatusRequest( uint256 const & _transactionHash, common::CMediumFilter< NodeResponses > * _medium )
	: common::CRequest< NodeResponses >( _medium )
	, m_transactionHash( _transactionHash )
{
}

void
CTransactionStatusRequest::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< NodeResponses > *
CTransactionStatusRequest::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}

void
CTransactionSendRequest::accept( CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

CTransactionSendRequest::CTransactionSendRequest( CTransaction const & _transaction, common::CMediumFilter< NodeResponses > * _medium )
	: common::CRequest< NodeResponses >( _medium )
	, m_transaction( _transaction )
{
}

common::CMediumFilter< NodeResponses > *
CTransactionSendRequest::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}


}

