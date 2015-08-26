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

#include "configureClientActionHadler.h"
#include "serialize.h"

using namespace common;

namespace client
{

struct CTransactionStatus;

struct CPrepareAndSendTransaction : boost::statechart::state< CPrepareAndSendTransaction, CSendTransactionAction >
{
	CPrepareAndSendTransaction( my_context ctx ) : my_base( ctx )
	{
		context< CSendTransactionAction >().dropRequests();
		context< CSendTransactionAction >().addRequest( new CTransactionSendRequest( context< CSendTransactionAction >().getTransaction(), new CMediumClassFilter( RequestKind::Transaction, 1 ) ) );
	}
//  ack here
	boost::statechart::result react( common::CPending const & _pending )
	{
		context< CSendTransactionAction >().setProcessingTrackerPtr( _pending.m_networkPtr );
		return discard_event();
	}

	boost::statechart::result react( CTransactionAckEvent const & _transactionSendAck )
	{
// todo, check status and validity of the transaction propagated
		if ( _transactionSendAck.m_status == common::TransactionsStatus::Validated )
		{
			CClientControl::getInstance()->addTransactionToModel( _transactionSendAck.m_transactionSend );
			context< CSendTransactionAction >().setValidatedTransactionHash( _transactionSendAck.m_transactionSend.GetHash() );
			return transit< CTransactionStatus >();
		}
		else
		{
			context< CSendTransactionAction >().dropRequests();
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	  boost::statechart::custom_reaction< common::CPending >
	, boost::statechart::custom_reaction< CTransactionAckEvent >
	> reactions;
};

struct CTransactionStatus : boost::statechart::state< CTransactionStatus, CSendTransactionAction >
{
	CTransactionStatus( my_context ctx ) : my_base( ctx )
	{
		common::CClientMediumFilter * filter =
				CTrackerLocalRanking::getInstance()->determinedTrackersCount() > 1 ?
											(common::CClientMediumFilter *)new CMediumClassWithExceptionFilter( context< CSendTransactionAction >().getProcessingTrackerPtr(), RequestKind::TransactionStatus, 1 )
										  : (common::CClientMediumFilter *)new CMediumClassFilter( RequestKind::Transaction, 1 );

		context< CSendTransactionAction >().dropRequests();
		context< CSendTransactionAction >().addRequest(
					new CTransactionStatusRequest(
						  context< CSendTransactionAction >().getValidatedTransactionHash()
						, filter
						) );
	}


	boost::statechart::result react( common::CPending const & _pending )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CTransactionStatus const & _transactionStats )
	{
		if ( _transactionStats.m_status == common::TransactionsStatus::Confirmed )
		{
			context< CSendTransactionAction >().setExit();
		}
		else if ( _transactionStats.m_status == common::TransactionsStatus::Unconfirmed )
		{
			context< CSendTransactionAction >().dropRequests();
			context< CSendTransactionAction >().addRequest(
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
{
	initiate();
}

void
CSendTransactionAction::accept( common::CSetResponseVisitor< common::CClientTypes > & _visitor )
{
	_visitor.visit( *this );
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
