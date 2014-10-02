// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MONITOR_CONTROLLER_H
#define MONITOR_CONTROLLER_H

#include <boost/statechart/state_machine.hpp>

namespace monitor
{

struct CMonitorInitialSynchronization;

class CMonitorController : public boost::statechart::state_machine< CMonitorController, CMonitorInitialSynchronization >
{
public:
	static CMonitorController* getInstance();

private:
	CMonitorController();

private:
	static CMonitorController * ms_instance;
};

}

#endif // MONITOR_CONTROLLER_H
