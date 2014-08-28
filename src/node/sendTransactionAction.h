// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEND_TRANSACTION_ACTION_H
#define SEND_TRANSACTION_ACTION_H

#include <boost/optional.hpp>

#include <boost/statechart/state_machine.hpp>

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

namespace client
{

struct CPrepareAndSendTransaction;

class CSendTransactionAction : public common::CAction< NodeResponses >, public  boost::statechart::state_machine< CSendTransactionAction, CPrepareAndSendTransaction >
{
public:
	CSendTransactionAction( const CTransaction & _Transaction );

	void accept( common::CSetResponseVisitor< NodeResponses > & _visitor );

	common::CRequest< NodeResponses > * execute();

	void setRequest( common::CRequest< NodeResponses > * _request );

	CTransaction const & getTransaction() const;
private:
	CTransaction m_transaction;

	common::ActionStatus::Enum m_actionStatus;
	
	common::CRequest< NodeResponses > * m_request;
};

struct CTransactionStatusRequest : public common::CRequest< NodeResponses >
{
public:
	CTransactionStatusRequest( uint256 const & _token, common::CMediumFilter< NodeResponses > * _medium );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;
	uint256 m_token;
};

struct CTransactionSendRequest : public common::CRequest< NodeResponses >
{
public:
	CTransactionSendRequest( CTransaction const & _transaction, common::CMediumFilter< NodeResponses > * _medium );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;
	CTransaction m_transaction;
};


}

#endif // SEND_TRANSACTION_ACTION_H
