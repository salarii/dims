// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ENTER_NETWORK_ACTION_H
#define ENTER_NETWORK_ACTION_H

#include "common/action.h"

#include <boost/statechart/state_machine.hpp>

namespace monitor
{

struct CEnterNetworkInitial;

class CEnterNetworkAction : public common::CAction< common::CMonitorTypes >, public  boost::statechart::state_machine< CEnterNetworkAction, CEnterNetworkInitial >
{
public:
	CEnterNetworkAction( uint256 const & _actionKey, uintptr_t _nodePtr );

	virtual void accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor );

	uintptr_t getNodePtr() const { return m_nodePtr; }
private:
	uintptr_t m_nodePtr;
};

}

#endif // ENTER_NETWORK_ACTION_H
