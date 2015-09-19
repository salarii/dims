// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcserver.h"

#include "common/events.h"

#include "monitor/controller.h"
#include "monitor/registerRpcHooks.h"

namespace monitor
{

std::string
getStatus()
{
	CController::getInstance()->process_event( common::CUpdateStatus() );

	return CController::getInstance()->getStatusMessage();
}

void registerHooks()
{
	SatusHook.connect( &getStatus );
}

}




