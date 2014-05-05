// Copyright (c) 2014 Ratcoin dev-team
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

	virtual int getKind();

	std::vector< CTransaction > const & getTransactions() const;
private:
	std::vector< CTransaction > const & m_transactions;
};


}

#endif // VALIDATE_TRANSACTIONS_REQUEST_H
