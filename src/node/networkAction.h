// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NETWORK_ACTION_H
#define NETWORK_ACTION_H

#include "actionHandler.h"
#include "tracker/validationManager.h"

namespace node
{

class CSetResponseVisitor;


struct ActionStatus
{
	enum Enum
	{
		 Unprepared
		,InProgress
	};
};

class CSendTransaction : public CAction
{
public:
	CSendTransaction( const CTransaction & _Transaction );

	void accept( CSetResponseVisitor & _visitor );

	CRequest * execute();

	void setTransactionStatus( self::TransactionsStatus::Enum const _transactionStatus );

	void setTransactionToken( uint256 const & _token );
private:
	ActionStatus::Enum m_actionStatus;
	CTransaction m_transaction;
	self::TransactionsStatus::Enum m_status;
	uint256 m_token;
	
};

struct CTransactionStatusRequest : public CRequest
{
public:
	CTransactionStatusRequest( uint256 const & _token );
	void serialize( CBufferAsStream & _bufferStream );
	uint256 m_token;
};

struct CTransactionSendRequest : public CRequest
{
public:
	CTransactionSendRequest( CTransaction const & _transaction );
	void serialize( CBufferAsStream & _bufferStream );
	CTransaction m_transaction;
};


}

#endif