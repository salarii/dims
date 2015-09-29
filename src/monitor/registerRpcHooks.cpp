// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcserver.h"

#include "base58.h"
#include "common/events.h"
#include "common/actionHandler.h"
#include "common/authenticationProvider.h"

#include "monitor/controller.h"
#include "monitor/registerRpcHooks.h"
#include "monitor/reputationTracer.h"
#include "monitor/enterNetworkAction.h"

namespace monitor
{

std::string
getStatus()
{
	CController::getInstance()->process_event( common::CUpdateStatus() );

	return CController::getInstance()->getStatusMessage();
}

std::string enterNetwork( std::string const & _key )
{
	CKeyID keyId;

	CNodeAddress nodeAddress(_key);

	if ( !nodeAddress.GetKeyID( keyId ) )
		goto WrongKey;

	uintptr_t nodeIndicator;
	if ( !CReputationTracker::getInstance()->getKeyToNode( keyId, nodeIndicator ) )
		goto NotPresent;

	common::CActionHandler::getInstance()->executeAction( new CEnterNetworkAction( nodeIndicator ) );
	return "registration in progress";

WrongKey:
		return "monitor with specified number not present";
NotPresent:
	return "monitor with specified number not present";
}

std::string
selfAddress()
{
	CMnemonicAddress address;

	address.Set( common::CAuthenticationProvider::getInstance()->getMyKey().GetID() );

	return address.ToString();
}

void registerHooks()
{
	SatusHook.connect( &getStatus );

	SelfAddress.connect( &selfAddress );

	EnterNetworkHook.connect( &enterNetwork );
}

}
