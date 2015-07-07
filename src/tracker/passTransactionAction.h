// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PASS_TRANSACTION_ACTION_H
#define PASS_TRANSACTION_ACTION_H

#include "common/scheduleAbleAction.h"

#include "configureTrackerActionHandler.h"

namespace tracker
{

struct CInitial;

class CPassTransactionAction : public common::CScheduleAbleAction< common::CTrackerTypes >, public  boost::statechart::state_machine< CPassTransactionAction, CInitial >
{
public:
	CPassTransactionAction( CTransaction const & _transaction );

	virtual void accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor );

	CTransaction const & getTransaction() const{ return m_transaction; }
private:
	CTransaction m_transaction;
};

}

#endif // PASS_TRANSACTION_ACTION_H
