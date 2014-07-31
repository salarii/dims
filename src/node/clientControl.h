#ifndef CLIENT_CONTROL_H
#define CLIENT_CONTROL_H

#include <boost/statechart/state_machine.hpp>

namespace client
{

struct CInitialClient;

class CClientControl : public boost::statechart::state_machine< CClientControl, CInitialClient >
{
public:
	static CTrackerController* getInstance();
private:
	CClientControl();

private:
	static CClientControl * ms_instance;
};
//ask  dns for  ips  - create  in  connect  action

// if  there  are trackers ask  them  for  info  about  network

// there are  monitor  reload  tracker  info  every  few  seconds

//

}

#endif // CLIENT_CONTROL_H
