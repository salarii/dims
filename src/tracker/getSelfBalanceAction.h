// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GET_SELF_BALANCE_ACTION_H
#define GET_SELF_BALANCE_ACTION_H

#include "common/types.h"
#include "common/scheduleAbleAction.h"

#include <boost/statechart/state_machine.hpp>
//this  action  needs  to recover self  balance

namespace tracker
{

struct CInitialState;

class CGetBalanceFromNetworkAction : public common::CScheduleAbleAction< common::CTrackerTypes >, public  boost::statechart::state_machine< CGetBalanceFromNetworkAction, CInitialState >
{
public:
	CGetBalanceFromNetworkAction();

	virtual void accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor );

};

}

#endif // GET_SELF_BALANCE_ACTION_H
