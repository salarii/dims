// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UPDATE_NETWORK_DATA_ACTION_H
#define UPDATE_NETWORK_DATA_ACTION_H

#include "common/action.h"
#include "common/filters.h"

#include <boost/statechart/state_machine.hpp>

#include "protocol.h"

namespace monitor
{

struct CUpdateNetworkData;

class CUpdateNetworkDataAction : public common::CAction, public  boost::statechart::state_machine< CUpdateNetworkDataAction, CUpdateNetworkData >
{
public:
	CUpdateNetworkDataAction( uint256 const & _actionKey );

	virtual void accept( common::CSetResponseVisitor & _visitor );
};


}

#endif // UPDATE_NETWORK_DATA_ACTION_H
