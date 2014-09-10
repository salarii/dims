// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VALIDATE_TRANSACTIONS_REQUEST_H
#define VALIDATE_TRANSACTIONS_REQUEST_H

#include "core.h"
#include "common/request.h"
#include "configureTrackerActionHandler.h"

namespace tracker
{

class  CValidateTransactionsRequest : public common::CRequest< TrackerResponses >
{
public:
	CValidateTransactionsRequest( std::vector< CTransaction > const & _transactions );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	std::vector< CTransaction > const & getTransactions() const;
private:
	std::vector< CTransaction > const m_transactions;
};

class CTransactionsPropagationRequest : public common::CRequest< TrackerResponses >
{
public:
	CTransactionsPropagationRequest( std::vector< CTransaction > const & _transactions, uint256 const & _actionKey,common::CMediumFilter< TrackerResponses > * _mediumFilter );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	uint256 getActionKey() const;

	std::vector< CTransaction > const & getTransactions() const;
private:
	uint256 const m_actionKey;

	std::vector< CTransaction > const m_transactions;
};


class  CTransactionsKnownRequest : public common::CRequest< TrackerResponses >
{
public:
	CTransactionsKnownRequest();

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;
};

class  CTransactionsAckRequest : public common::CRequest< TrackerResponses >
{
public:
	CTransactionsAckRequest();

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

};

class  CTransactionsDoublespendRequest : public common::CRequest< TrackerResponses >
{
public:
	CTransactionsDoublespendRequest();

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;
};

class  CTransactionsNotOkRequest : public common::CRequest< TrackerResponses >
{
public:
	CTransactionsNotOkRequest();

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;
};


}

#endif // VALIDATE_TRANSACTIONS_REQUEST_H
