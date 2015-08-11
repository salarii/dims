// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SYNCHRONIZATION_ACTION_H
#define SYNCHRONIZATION_ACTION_H

#include "common/action.h"

#include <boost/statechart/state_machine.hpp>

#include "monitor/configureMonitorActionHandler.h"

namespace monitor
{

struct CUninitiated;

class CSynchronizationAction : public common::CAction< common::CMonitorTypes >, public  boost::statechart::state_machine< CSynchronizationAction, CUninitiated >
{
public:
	CSynchronizationAction();

	CSynchronizationAction( uint256 const & _actionKey, uintptr_t _nodeIndicator, uint64_t _timeStamp );

	virtual void accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor );

	void clear();

	void setNodeIdentifier( unsigned int _nodeIdentifier );
	
	unsigned long long getNodeIdentifier() const;

	bool isRequestInitialized() const;
private:
	uint256 m_currentHash;

	uint64_t m_timeStamp;

	uintptr_t m_nodeIdentifier;
};

}

#endif // SYNCHRONIZATION_ACTION_H
