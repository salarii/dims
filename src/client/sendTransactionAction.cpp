// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/transition.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/nodeMessages.h"
#include "common/setResponseVisitor.h"
#include "common/medium.h"
#include "common/requests.h"

#include "client/sendTransactionAction.h"
#include "client/filters.h"
#include "client/requests.h"
#include "client/events.h"
#include "client/control.h"

#include "serialize.h"

using namespace common;

namespace client
{

struct CTransactionStatus;

struct CPrepareAndSendTransaction : boost::statechart::state< CPrepareAndSendTransaction, CSendTransactionAction >
{
	CPrepareAndSendTransaction( my_context ctx ) : my_base( ctx )
	{
		context< CSendTransactionAction >().forgetRequests();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CMainRequestType::Transaction
					, new CMediumClassFilter( ClientMediums::Trackers, 1 ) );

		request->addPayload( common::CClientTransactionSend(context< CSendTransactionAction >().getTransaction()) );

		context< CSendTransactionAction >().addRequest( request );
	}

	boost::statechart::result react( common::CClientMessageResponse const & _message )
	{
		common::CTransactionAck transactionAckData;
		convertClientPayload( _message.m_clientMessage, transactionAckData );

		if ( transactionAckData.m_status == (int)common::TransactionsStatus::Validated )
		{
			CClientControl::getInstance()->addTransactionToModel( transactionAckData.m_transaction );
			context< CSendTransactionAction >().setTransaction( transactionAckData.m_transaction );
			return transit< CTransactionStatus >();
		}
		else
		{
			context< CSendTransactionAction >().forgetRequests();
		}

		return discard_event();
	}

	typedef boost::mpl::list<
		boost::statechart::custom_reaction< common::CClientMessageResponse >
	> reactions;
};

struct CTransactionStatus : boost::statechart::state< CTransactionStatus, CSendTransactionAction >
{
	CTransactionStatus( my_context ctx ) : my_base( ctx )
	{
		common::CMediumFilter * filter =
				CTrackerLocalRanking::getInstance()->determinedTrackersCount() > 1 ?
											(common::CMediumFilter *)new CMediumClassWithExceptionFilter( context< CSendTransactionAction >().getProcessingTrackerPtr(), ClientMediums::Trackers, 1 )
										  : (common::CMediumFilter *)new CMediumClassFilter( ClientMediums::Trackers, 1 );

		context< CSendTransactionAction >().forgetRequests();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CMainRequestType::TransactionStatusReq
					, filter );

		request->addPayload( common::CClientTransactionStatusAsk(context< CSendTransactionAction >().getTransaction().GetHash()) );

		context< CSendTransactionAction >().addRequest( request );
	}

	boost::statechart::result react( common::CClientMessageResponse const & _message )
	{
		common::CTransactionStatus transactionStatus;
		convertClientPayload( _message.m_clientMessage, transactionStatus );
		if ( transactionStatus.m_status == (int)common::TransactionsStatus::Confirmed )
		{
			CClientControl::getInstance()->transactionAddmited(
						context< CSendTransactionAction >().getInitialTransactionHash()
						, context< CSendTransactionAction >().getTransaction() );

			context< CSendTransactionAction >().setExit();
		}
		else if ( transactionStatus.m_status == common::TransactionsStatus::Unconfirmed )
		{
			context< CSendTransactionAction >().forgetRequests();

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CMainRequestType::TransactionStatusReq
						, new CMediumClassWithExceptionFilter( _message.m_nodePtr, ClientMediums::Trackers, 1 ) );

			request->addPayload( common::CClientTransactionStatusAsk(context< CSendTransactionAction >().getTransaction().GetHash()) );

			context< CSendTransactionAction >().addRequest( request );
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CClientMessageResponse >
	> reactions;
};

CSendTransactionAction::CSendTransactionAction( const CTransaction & _transaction )
	: CAction()
	, m_transaction( _transaction )
{
	m_initialTransactionHash = m_transaction.GetHash();
	initiate();
}

void
CSendTransactionAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

CTransaction const &
CSendTransactionAction::getTransaction() const
{
	return m_transaction;
}

void
CSendTransactionAction::setTransaction( CTransaction const & _tx )
{
	m_transaction = _tx;
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

}
