// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include <string>

#include "rpcserver.h"
#include "base58.h"
#include "wallet.h"

#include "common/events.h"
#include "common/actionHandler.h"
#include "common/authenticationProvider.h"

#include "tracker/controllerEvents.h"
#include "tracker/controller.h"
#include "tracker/registerRpcHooks.h"
#include "tracker/trackerNodesManager.h"
#include "tracker/registerAction.h"
#include "tracker/connectNetworkAction.h"
#include "tracker/passTransactionAction.h"

namespace tracker
{

std::string
getStatus()
{
	CController::getInstance()->process_event( common::CUpdateStatus() );

	return CController::getInstance()->getStatusMessage();
}

std::string sendCoins( std::string const & _key, int _amount )
{
	CKeyID keyId;

	CMnemonicAddress mnemonicAddress( _key );

	if ( !mnemonicAddress.GetKeyID( keyId ) )
		return "invalid key specified";

	common::CActionHandler::getInstance()->executeAction(
				new CPassTransactionAction( keyId, _amount ) );

	return "executing";
}

std::string registerInNetwork( std::string const & _key )
{
	CKeyID keyId;

	CNodeAddress nodeAddress(_key);

	if ( !nodeAddress.GetKeyID( keyId ) )
		return "monitor with specified number not present";

	uintptr_t nodeIndicator;
	if ( !CTrackerNodesManager::getInstance()->getKeyToNode( keyId, nodeIndicator ) )
		return "monitor with specified number not present";

	common::CActionHandler::getInstance()->executeAction( new CRegisterAction( nodeIndicator ) );
	return "registration in progress";
}

std::string
connectNetwork()
{
	if ( CController::getInstance()->isConnected() )
			return "tracker already connected";

	common::CActionHandler::getInstance()->executeAction( new CConnectNetworkAction() );
	return "connection in progress";
}

std::string
selfAddress()
{
	CMnemonicAddress address;

	address.Set( common::CAuthenticationProvider::getInstance()->getMyKey().GetID() );

	double coins = CWallet::getInstance()->AvailableCoinsAmount( common::CAuthenticationProvider::getInstance()->getMyKey().GetID() );
	coins /= 100;

	std::ostringstream coinAmount;
	coinAmount.precision (2);
	coinAmount << coins;

	return address.ToString() + "\n\n" + "available coins: " + coinAmount.str();
}

void registerHooks()
{
	SatusHook.connect( &getStatus );
	RegisterInNetworkHook.connect( &registerInNetwork );
	SelfAddress.connect( &selfAddress );
	ConnectNetworkHook.connect( &connectNetwork );
	SendCoins.connect( &sendCoins );
}

}
