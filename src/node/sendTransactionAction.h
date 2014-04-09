// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_TRANSACTION_ACTION_H
#define SEND_TRANSACTION_ACTION_H

#include <boost/optional.hpp>

#include "common/transactionStatus.h"
#include "action.h"
#include "request.h"

#include "core.h"

namespace node
{

class CSetResponseVisitor;


class CSendTransactionAction : public CAction
{
public:
	CSendTransactionAction( const CTransaction & _Transaction );

	void accept( CSetResponseVisitor & _visitor );

	CRequest * execute();

	void setTransactionStatus( boost::optional< common::TransactionsStatus::Enum > const _transactionStatus );

	void setTransactionToken(  boost::optional< uint256 > const & _token );
private:
	ActionStatus::Enum m_actionStatus;
	CTransaction m_transaction;
	common::TransactionsStatus::Enum m_status;
	uint256 m_token;
	
};

struct CTransactionStatusRequest : public CRequest
{
public:
	CTransactionStatusRequest( uint256 const & _token );
	void serialize( CBufferAsStream & _bufferStream ) const;
	RequestKind::Enum getKind() const;
	uint256 m_token;
};

struct CTransactionSendRequest : public CRequest
{
public:
	CTransactionSendRequest( CTransaction const & _transaction );
	void serialize( CBufferAsStream & _bufferStream ) const;
	RequestKind::Enum getKind() const;
	CTransaction m_transaction;
};


}

#endif // SEND_TRANSACTION_ACTION_H