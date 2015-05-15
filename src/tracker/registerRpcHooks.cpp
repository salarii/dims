// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcserver.h"

#include "common/commonEvents.h"

#include "tracker/trackerControllerEvents.h"
#include "tracker/trackerController.h"
#include "tracker/registerRpcHooks.h"

namespace tracker
{

std::string
getStatus()
{
	CTrackerController::getInstance()->process_event( common::CUpdateStatus() );

	return CTrackerController::getInstance()->getStatusMessage();
}

void registerInNetwork()
{}

void connectNetwork()
{}

void registerHooks()
{
	SatusHook.connect( &getStatus );
	RegisterInNetworkHook.connect( &registerInNetwork );
	ConnectNetworkHook.connect( &connectNetwork );
}

}
