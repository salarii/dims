// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COPY_TRANSACTION_STORAGE_ACTION_H
#define COPY_TRANSACTION_STORAGE_ACTION_H

#include "common/scheduleAbleAction.h"
#include "common/types.h"

#include "configureMonitorActionHandler.h"

#include <boost/statechart/state_machine.hpp>

namespace monitor
{

struct CUninitiated;
/*
provide  about  stored  copy
initiate  new  copy or  provide  info  about  next  time
*/
class CCopyTransactionStorageAction : public common::CScheduleAbleAction< common::CMonitorTypes >, public  boost::statechart::state_machine< CCopyTransactionStorageAction, CUninitiated >
{
public:
	CCopyTransactionStorageAction();

	virtual void accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor );

	void clear(){};

	uintptr_t getNodeIdentifier() const{ return m_ptr; }
private:
	uintptr_t m_ptr;
};

}

#endif // COPY_TRANSACTION_STORAGE_ACTION_H
