// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VALIDATE_TRANSACTIONS_ACTION_H
#define VALIDATE_TRANSACTIONS_ACTION_H

#include <boost/statechart/state_machine.hpp>

#include "common/action.h"
#include "configureTrackerActionHandler.h"

namespace tracker
{

struct CInitial;

class CValidateTransactionsAction : public common::CAction< TrackerResponses >,public boost::statechart::state_machine< CValidateTransactionsAction, CInitial >
{
public:
	friend class CInitial;
public:
	CValidateTransactionsAction( std::vector< CTransaction > const & _transactions );
	virtual common::CRequest< TrackerResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< TrackerResponses > & _visitor );
private:
	common::CRequest< TrackerResponses >* m_request;
	std::vector< CTransaction > m_transactions;
};

}

#endif // VALIDATE_TRANSACTIONS_ACTION_H
