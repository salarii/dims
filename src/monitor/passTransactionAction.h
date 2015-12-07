// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PASS_TRANSACTION_ACTION_H
#define PASS_TRANSACTION_ACTION_H

#include <boost/statechart/state_machine.hpp>

#include "common/scheduleAbleAction.h"

namespace monitor
{

struct CProcessAsClient;

class CPassTransactionAction : public common::CScheduleAbleAction, public  boost::statechart::state_machine< CPassTransactionAction, CProcessAsClient >
{
public:
	CPassTransactionAction( uint256 const & _actionKey );

	CPassTransactionAction( CKeyID const & _keyId, int64_t _amount );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	CKeyID getKeyId() const{ return m_keyId; }

	int64_t getAmount() const{ return m_amount; }
public:
	uint256 m_hash;

	common::CTrackerInfo m_servicingTracker;

	CTransaction m_transaction;
private:
	CKeyID const m_keyId;

	int64_t m_amount;
};

}

#endif // PASS_TRANSACTION_ACTION_H
