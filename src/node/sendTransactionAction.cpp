// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/transition.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "sendTransactionAction.h"
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

struct CCheckTransactionStatus;

struct CPrepareAndSendTransaction : boost::statechart::state< CPrepareAndSendTransaction, CSendTransactionAction >
{
	CPrepareAndSendTransaction( my_context ctx ) : my_base( ctx )
	{
		context< CSendTransactionAction >().setRequest( new CTransactionSendRequest( context< CSendTransactionAction >().getTransaction(), new CMediumClassFilter( RequestKind::Transaction, 1 ) ) );
	}
//  ack here
	boost::statechart::result react( common::CPending const & _pending )
	{
		context< CSendTransactionAction >().setProcessingTrackerPtr( _pending.m_networkPtr );
		context< CSendTransactionAction >().setRequest( new CInfoRequestContinue( _pending.m_token, new CSpecificMediumFilter( _pending.m_networkPtr ) ) );
		return discard_event();
	}

	boost::statechart::result react( CTransactionAckEvent const & _transactionSendAck )
	{
// todo, check status and validity of the transaction propagated
		if ( _transactionSendAck.m_status == common::TransactionsStatus::Validated )
		{
			CClientControl::getInstance()->addTransactionToModel( _transactionSendAck.m_transactionSend );
			context< CSendTransactionAction >().setValidatedTransactionHash( _transactionSendAck.m_transactionSend.GetHash() );
			return transit< CCheckTransactionStatus >();
		}
		else
		{
			context< CSendTransactionAction >().setRequest( 0 );
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction< common::CPending >
	, boost::statechart::custom_reaction< CTransactionAckEvent >
	> reactions;
};

struct CCheckTransactionStatus : boost::statechart::state< CCheckTransactionStatus, CSendTransactionAction >
{
	CCheckTransactionStatus( my_context ctx ) : my_base( ctx )
	{
		context< CSendTransactionAction >().setRequest(
					new CTransactionStatusRequest(
						  context< CSendTransactionAction >().getValidatedTransactionHash()
						, new CMediumClassWithExceptionFilter( context< CSendTransactionAction >().getProcessingTrackerPtr(), RequestKind::TransactionStatus, 1 ) ) );
	}

	boost::statechart::result react( common::CPending const & _pending )
	{
		context< CSendTransactionAction >().setRequest( new CInfoRequestContinue( _pending.m_token, new CSpecificMediumFilter( _pending.m_networkPtr ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CTransactionStatus const & _transactionStats )
	{
		if ( _transactionStats.m_status == common::TransactionsStatus::Confirmed )
		{
			context< CSendTransactionAction >().setRequest( 0 );
		}
		else if ( _transactionStats.m_status == common::TransactionsStatus::Unconfirmed )
		{
			context< CSendTransactionAction >().setRequest(
						new CTransactionStatusRequest(
							  context< CSendTransactionAction >().getValidatedTransactionHash()
							, new CMediumClassWithExceptionFilter( context< CSendTransactionAction >().getProcessingTrackerPtr(), RequestKind::TransactionStatus, 1 ) ) );
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction<  common::CPending >
	, boost::statechart::custom_reaction< common::CTransactionStatus >
	> reactions;
};

CSendTransactionAction::CSendTransactionAction( const CTransaction & _transaction )
	: CAction()
	, m_transaction( _transaction )
	, m_actionStatus( common::ActionStatus::Unprepared )
{
	initiate();
}

void
CSendTransactionAction::accept( common::CSetResponseVisitor< NodeResponses > & _visitor )
{
	_visitor.visit( *this );
}


CRequest< NodeResponses > *
CSendTransactionAction::execute()
{
	return m_request;
}

void
CSendTransactionAction::setRequest( common::CRequest< NodeResponses > * _request )
{
	m_request = _request;
}

CTransaction const &
CSendTransactionAction::getTransaction() const
{
	return m_transaction;
}

void
CSendTransactionAction::setProcessingTrackerPtr( uintptr_t _ptr )
{
	m_processingTrackerPtr = _ptr;
}

uintptr_t
CSendTransactionAction::getProcessingTrackerPtr() const
{
	return m_processingTrackerPtr;
}

void
CSendTransactionAction::setValidatedTransactionHash( uint256 _hash )
{
	m_validatedTransactionHash = _hash;
}

uint256
CSendTransactionAction::getValidatedTransactionHash() const
{
	return m_validatedTransactionHash;
}


}
