// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLIENT_CONTROL_H
#define CLIENT_CONTROL_H

#include <boost/statechart/state_machine.hpp>

namespace client
{

struct CUninitiatedClient;

class CClientControl : public boost::statechart::state_machine< CClientControl, CUninitiatedClient >
{
public:
	static CClientControl* getInstance();
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
