// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PROVIDE_INFO_ACTION_H
#define PROVIDE_INFO_ACTION_H

#include "common/scheduleAbleAction.h"
#include "common/filters.h"

#include "configureMonitorActionHandler.h"

#include <boost/statechart/state_machine.hpp>

#include "protocol.h"

/*
current communication protocol is ineffective
consider using ack  request after  every successful message  reception
*/

namespace monitor
{

struct CProvideInfo;
// rework  this  sooner  or later

class CProvideInfoAction : public common::CScheduleAbleAction< common::CMonitorTypes >, public  boost::statechart::state_machine< CProvideInfoAction, CProvideInfo >
{
public:
	CProvideInfoAction( uint256 const & _actionKey, uintptr_t _nodeIndicator );

	virtual void accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor );

	uintptr_t getNodeIndicator() const;

	uint256 getInfoRequestKey() const{ return m_infoRequestKey; }

	void setInfoRequestKey( uint256 const & _key ){ m_infoRequestKey = _key; }

	~CProvideInfoAction(){};
private:
	common::CCommunicationRegisterObject m_registerObject;

	uint256 m_infoRequestKey;

	uintptr_t m_nodeIndicator;
};

}

#endif // PROVIDE_INFO_ACTION_H
