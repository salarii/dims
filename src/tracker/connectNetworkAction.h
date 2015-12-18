// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECT_NETWORK_ACTION_H
#define CONNECT_NETWORK_ACTION_H

#include "common/action.h"
#include "common/filters.h"

#include <boost/statechart/state_machine.hpp>

namespace tracker
{
struct CInitialConnect;

class CConnectNetworkAction : public common::CAction, public  boost::statechart::state_machine< CConnectNetworkAction, CInitialConnect >
{
public:
	CConnectNetworkAction();

	virtual void accept( common::CSetResponseVisitor & _visitor );
};


}

#endif // CONNECT_NETWORK_ACTION_H
