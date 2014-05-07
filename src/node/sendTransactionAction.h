// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_TRANSACTION_ACTION_H
#define SEND_TRANSACTION_ACTION_H

#include <boost/optional.hpp>


#include "common/action.h"
#include "common/request.h"
#include "configureNodeActionHadler.h"
#include "core.h"
#include "common/transactionStatus.h"

namespace common
{

template < class _RequestResponses >
class CSetResponseVisitor;

}

namespace node
{

class CSendTransactionAction : public common::CAction< NodeResponses >
{
public:
	CSendTransactionAction( const CTransaction & _Transaction );

	void accept( common::CSetResponseVisitor< NodeResponses > & _visitor );

	common::CRequest< NodeResponses > * execute();

	void setTransactionStatus( boost::optional< common::TransactionsStatus::Enum > const _transactionStatus );

	void setTransactionToken(  boost::optional< uint256 > const & _token );

private:
	CTransaction m_transaction;
	common::TransactionsStatus::Enum m_status;
	uint256 m_token;
	
};

struct CTransactionStatusRequest : public common::CRequest< NodeResponses >
{
public:
	CTransactionStatusRequest( uint256 const & _token );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	int getKind() const;
	uint256 m_token;
};

struct CTransactionSendRequest : public common::CRequest< NodeResponses >
{
public:
	CTransactionSendRequest( CTransaction const & _transaction );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	int getKind() const;
	CTransaction m_transaction;
};


}

#endif // SEND_TRANSACTION_ACTION_H
