// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UPDATE_DATA_ACTION_H
#define UPDATE_DATA_ACTION_H

#include "common/action.h"
#include "common/filters.h"

#include "configureMonitorActionHandler.h"

#include <boost/statechart/state_machine.hpp>

#include "protocol.h"

namespace monitor
{

struct CAskForUpdate;

// rework  this  sooner  or later

class CUpdateDataAction : public common::CAction< MonitorResponses >, public  boost::statechart::state_machine< CUpdateDataAction, CAskForUpdate >, public common::CCommunicationAction
{
public:
	CUpdateDataAction();

	virtual common::CRequest< MonitorResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< MonitorResponses > & _visitor );

	void setRequest( common::CRequest< MonitorResponses >* _request );

	common::CRequest< MonitorResponses > const * getRequest() const;

	~CUpdateDataAction(){};
private:
	common::CRequest< MonitorResponses >* m_request;
};


}

#endif // UPDATE_DATA_ACTION_H
