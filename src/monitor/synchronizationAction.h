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

struct CSwitchToSynchronized : boost::statechart::event< CSwitchToSynchronized >
{
};

class CSynchronizationAction : public common::CAction< common::CMonitorTypes >, public  boost::statechart::state_machine< CSynchronizationAction, CUninitiated >
{
public:
	CSynchronizationAction();

	CSynchronizationAction( uint256 const & _id, uint256 const & _actionKey, uintptr_t _nodeIndicator );

	virtual void accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor );

	void clear();

	void setNodeIdentifier( unsigned int _nodeIdentifier );
	
	unsigned long long getNodeIdentifier() const;

	bool isRequestInitialized() const;

	uint256 getRequestKey() const { return m_requestKey; }

	void setRequestKey( uint256 const & _requestKey ){ m_requestKey = _requestKey; }
private:
	uint256 m_requestKey;

	uintptr_t m_nodeIdentifier;
};

}

#endif // SYNCHRONIZATION_ACTION_H
