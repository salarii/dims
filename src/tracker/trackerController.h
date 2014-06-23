// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_CONTROLLER_H
#define TRACKER_CONTROLLER_H

#include <boost/statechart/state_machine.hpp>

namespace tracker
{
struct CStandAlone;

class CTrackerController : public boost::statechart::state_machine< CTrackerController, CStandAlone >
{
public:
	static CTrackerController* getInstance();
private:
	CTrackerController();

private:
	static CTrackerController * ms_instance;
};


}

#endif // TRACKER_CONTROLLER_H
