// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "net.h"
#include "rpcserver.h"

#include <stdint.h>
#include <boost/assign/list_of.hpp>
#include "json/json_spirit_utils.h"
#include "json/json_spirit_value.h"

#include "common/actionHandler.h"
#include "connectNodeAction.h"

namespace tracker
{

json_spirit::Value connectToTracker( json_spirit::Array const & params, bool fHelp)
{
	string strCommand;
	if (params.size() == 2)
		strCommand = params[1].get_str();
	if (fHelp || params.size() != 2 ||
		( strCommand != "add" ))
		throw runtime_error(
			"connecttotracker \"tracker\" \"add\"\n"
			"\nAttempts add tracker to the addTrackers list.\n"
			"Or try a connection to a tracker once.\n"
			"\nArguments:\n"
			"1. \"tracker\"     (string, required) The tracker\n"
			"2. \"command\"  (string, required) connecttotracker add to try a connection to the tracker once\n"
			"\nExamples:\n"
			+ HelpExampleCli("connecttotracker", "\"192.168.0.6:20020\" \"add\"")
			+ HelpExampleRpc("connecttotracker", "\"192.168.0.6:20020\", \"add\"")
		);

	string strNode = params[0].get_str();

	common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( (common::CAction< common::CTrackerTypes >*)new CConnectNodeAction( strNode ) );
	return json_spirit::Value::null;
}



}
