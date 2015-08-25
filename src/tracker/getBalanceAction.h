// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GET_BALANCE_ACTION_H
#define GET_BALANCE_ACTION_H

#include "configureTrackerActionHandler.h"
#include <boost/statechart/state_machine.hpp>

#include "common/scheduleAbleAction.h"
#include "common/types.h"

namespace tracker
{

struct CFindBalance;

class CGetBalanceAction : public common::CScheduleAbleAction< common::CTrackerTypes >, public boost::statechart::state_machine< CGetBalanceAction, CFindBalance >
{
public:
	CGetBalanceAction();

	CGetBalanceAction( uint160 const & _keyId, uint256 const & _hash );

	virtual void accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor );

	virtual void reset(){}

	uint256 getHash() const { return m_hash; }

	uint160 getKeyId() const { return m_keyId; }
private:
	uint160 const m_keyId;

	uint256 const m_hash;
};

}

#endif // GET_BALANCE_ACTION_H
