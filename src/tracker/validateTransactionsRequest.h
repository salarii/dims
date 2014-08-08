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

	virtual void accept( common::CMediumFilter< TrackerResponses > * _medium ) const;

	virtual common::CMediumFilter< TrackerResponses > * getMediumFilter() const;

	std::vector< CTransaction > const & getTransactions() const;
private:
	std::vector< CTransaction > const m_transactions;
};

class  CTransactionsKnownRequest : public common::CRequest< TrackerResponses >
{
public:
	CTransactionsKnownRequest();

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual common::CMediumFilter< TrackerResponses > * getMediumFilter() const;
};

class  CTransactionsAckRequest : public common::CRequest< TrackerResponses >
{
public:
	CTransactionsAckRequest();

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual common::CMediumFilter< TrackerResponses > * getMediumFilter() const;

};

class  CTransactionsDoublespendRequest : public common::CRequest< TrackerResponses >
{
public:
	CTransactionsDoublespendRequest();

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual common::CMediumFilter< TrackerResponses > * getMediumFilter() const;
};

class  CTransactionsNotOkRequest : public common::CRequest< TrackerResponses >
{
public:
	CTransactionsNotOkRequest();

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual common::CMediumFilter< TrackerResponses > * getMediumFilter() const;
};


}

#endif // VALIDATE_TRANSACTIONS_REQUEST_H
