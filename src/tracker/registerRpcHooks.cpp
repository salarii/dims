// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcserver.h"
#include "base58.h"

#include "common/commonEvents.h"
#include "common/actionHandler.h"
#include "common/authenticationProvider.h"

#include "tracker/controllerEvents.h"
#include "tracker/controller.h"
#include "tracker/registerRpcHooks.h"
#include "tracker/trackerNodesManager.h"
#include "tracker/registerAction.h"

namespace tracker
{

std::string
getStatus()
{
	CController::getInstance()->process_event( common::CUpdateStatus() );

	return CController::getInstance()->getStatusMessage();
}

std::string registerInNetwork( std::string const & _key )
{
	CKeyID keyId;

	CNodeAddress nodeAddress(_key);

	if ( !nodeAddress.GetKeyID( keyId ) )
		goto WrongKey;

	uintptr_t nodeIndicator;
	if ( !CTrackerNodesManager::getInstance()->getKeyToNode( keyId, nodeIndicator ) )
		goto NotPresent;

	common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( new CRegisterAction( nodeIndicator ) );
	return "registration in progress";

WrongKey:
		return "monitor with specified number not present";
NotPresent:
	return "monitor with specified number not present";
}

void aconnectNetwork()
{
	// synchronizatin  action

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
	RegisterInNetworkHook.connect( &registerInNetwork );
	SelfAddress.connect( &selfAddress );
//	ConnectNetworkHook.connect( &connectNetwork );
}

}
