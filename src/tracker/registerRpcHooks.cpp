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

	common::CValidNodeInfo validNodeInfo;
	if ( !CTrackerNodesManager::getInstance()->getNodeInfo( keyId, validNodeInfo ) )
		return "monitor with specified number not present";

	common::CActionHandler::getInstance()->executeAction( new CRegisterAction( validNodeInfo.m_publicKey ) );
	return "registration in progress";
}

std::string
networkInfo()
{
	std::string info;

	info += "Monitors: \n";
	BOOST_FOREACH( common::CValidNodeInfo const & allyMonitorData, CTrackerNodesManager::getInstance()->getNetworkMonitors() )
	{
		CNodeAddress monitor;
		monitor.Set( allyMonitorData.m_publicKey.GetID(), common::NodePrefix::Monitor );
		info += "key " + monitor.ToString() + " ip " + allyMonitorData.m_address.ToString() + "\n";
	}

	info += "Trackers: \n";
	BOOST_FOREACH( common::CValidNodeInfo const & allyTrackerData, CTrackerNodesManager::getInstance()->getNetworkTrackers() )
	{
		CNodeAddress tracker;
		tracker.Set( allyTrackerData.m_publicKey.GetID(), common::NodePrefix::Tracker );
		info += "key " + tracker.ToString() + " ip " + allyTrackerData.m_address.ToString() + "\n";
	}

	return info;
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

std::string
synchronizeBitcoin()
{
	CController::getInstance()->process_event( common::CSynchronizeBitcoinAsk() );

	return std::string("executing");
}

void registerHooks()
{
	SatusHook.connect( &getStatus );
	RegisterInNetworkHook.connect( &registerInNetwork );
	SelfAddress.connect( &selfAddress );
	ConnectNetworkHook.connect( &connectNetwork );
	SendCoins.connect( &sendCoins );
	SynchronizeBitcoin.connect( &synchronizeBitcoin );
	NetworkInfo.connect( &networkInfo );
}

}
