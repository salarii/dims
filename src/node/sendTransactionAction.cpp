// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sendTransactionAction.h"
#include "common/nodeMessages.h"
#include "common/setResponseVisitor.h"
#include "configureNodeActionHadler.h"
#include "common/medium.h"

#include "serialize.h"

using namespace common;

namespace node
{

CSendTransactionAction::CSendTransactionAction( const CTransaction & _transaction )
	: CAction()
	, m_transaction( _transaction )
{
}

void
CSendTransactionAction::setTransactionStatus( boost::optional< common::TransactionsStatus::Enum > const _transactionStatus )
{

}

void 
CSendTransactionAction::setInProgressToken( boost::optional< uint256 > const & _token )
{

}

void
CSendTransactionAction::accept( common::CSetResponseVisitor< NodeResponses > & _visitor )
{
	_visitor.visit( *this );
}


CRequest< NodeResponses > *
CSendTransactionAction::execute()
{
	if ( m_actionStatus == ActionStatus::Unprepared )
	{
		m_actionStatus = ActionStatus::InProgress;
		return new CTransactionSendRequest( m_transaction );
	}
	else
	{
		return 0;
		if ( m_status == common::TransactionsStatus::DoubleSpend );
			// double spend
		else if ( m_status == common::TransactionsStatus::Confirmed );
			//confirmed 
		else
			return new CTransactionStatusRequest( m_token );
	}

	//create  transaction  message 
	// send transaction  message 

//  ping  for transaction 

// update  status  accordingly 

// maybe   visitor  needed  to  load  message  data 
}


CTransactionStatusRequest::CTransactionStatusRequest( uint256 const & _token )
	: m_token( _token )
{
}

void
CTransactionStatusRequest::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

int
CTransactionStatusRequest::getKind() const
{
	return RequestKind::TransactionStatus;
}
/*
void
CTransactionStatusRequest::serialize( CBufferAsStream & _bufferStream ) const
{
	signed int infoReq =  CMainRequestType::TrackerInfoReq;
	_bufferStream << infoReq;
	_bufferStream << m_token;
}
*/
void
CTransactionSendRequest::accept( CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

CTransactionSendRequest::CTransactionSendRequest( CTransaction const & _transaction )
	: m_transaction( _transaction )
{
}

int CTransactionSendRequest::getKind() const
{
	return RequestKind::Transaction;
}


}
