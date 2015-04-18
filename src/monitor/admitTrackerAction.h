// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ADMIT_TRACKER_ACTION_H
#define ADMIT_TRACKER_ACTION_H

#include "common/action.h"
#include "common/types.h"

#include <boost/statechart/state_machine.hpp>

namespace monitor
{

struct CWaitForInfo;

class CAdmitTrackerAction : public common::CAction< common::CMonitorTypes >, public  boost::statechart::state_machine< CAdmitTrackerAction, CWaitForInfo >
{
public:
	CAdmitTrackerAction( uint256 const & _actionKey, uintptr_t _nodePtr );

	virtual void accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor );

	uintptr_t getNodePtr() const { return m_nodePtr; }

	~CAdmitTrackerAction(){};
private:
	common::CCommunicationRegisterObject m_registerObject;

	uintptr_t m_nodePtr;
};

}

#endif // ADMIT_TRACKER_ACTION_H
