// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PROVIDE_INFO_ACTION_H
#define PROVIDE_INFO_ACTION_H

#include "common/action.h"
#include "common/filters.h"

#include "configureTrackerActionHandler.h"

#include <boost/statechart/state_machine.hpp>

#include "protocol.h"

/*
current communication protocol is ineffective
consider using ack  request after  every successful message  reception
*/

namespace tracker
{


struct CProvideInfo;
// rework  this  sooner  or later

class CProvideInfoAction : public common::CAction< TrackerResponses >, public  boost::statechart::state_machine< CProvideInfoAction, CProvideInfo >, public common::CCommunicationAction
{
public:
	CProvideInfoAction( uint256 const & _actionKey, uintptr_t _nodeIndicator );

	virtual void accept( common::CSetResponseVisitor< TrackerResponses > & _visitor );

	uintptr_t getNodeIndicator()const;

	~CProvideInfoAction(){};
private:
	uintptr_t m_nodeIndicator;
};

}

#endif // PROVIDE_INFO_ACTION_H
