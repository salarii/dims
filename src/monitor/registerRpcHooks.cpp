// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcserver.h"

#include "common/commonEvents.h"

#include "monitor/monitorController.h"
#include "monitor/registerRpcHooks.h"

namespace monitor
{

std::string
getStatus()
{
	CMonitorController::getInstance()->process_event( common::CUpdateStatus() );

	return CMonitorController::getInstance()->getStatusMessage();
}

void registerHooks()
{
	SatusHook.connect( &getStatus );
}

}




