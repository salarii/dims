// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "rpcserver.h"
#include "wallet.h"
#include "base58.h"

#include "common/events.h"
#include "common/actionHandler.h"
#include "common/authenticationProvider.h"

#include "monitor/controller.h"
#include "monitor/registerRpcHooks.h"
#include "monitor/reputationTracer.h"
#include "monitor/enterNetworkAction.h"
#include "monitor/passTransactionAction.h"
#include "monitor/reputationControlAction.h"
#include "monitor/trackOriginAddressAction.h"

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
		return "monitor with specified number not present";

	uintptr_t nodeIndicator;
	if ( !CReputationTracker::getInstance()->getKeyToNode( keyId, nodeIndicator ) )
		return "monitor with specified number not present";

	CPubKey pubKey;
	if ( !CReputationTracker::getInstance()->getNodeToKey( nodeIndicator, pubKey ) )
		return "monitor with specified number not present";

	common::CActionHandler::getInstance()->executeAction( new CEnterNetworkAction( pubKey ) );
	return "registration in progress";
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

// this establishes stand alone network,  rethink name?
std::string
synchronizeBitcoin()
{
	CController::getInstance()->process_event( common::CSynchronizeBitcoinAsk() );

	return std::string("executing");
}

std::string
networkInfo()
{
	std::string info;

	info += "Monitors: \n";
	BOOST_FOREACH( common::CAllyMonitorData const & allyMonitorData, CReputationTracker::getInstance()->getAllyMonitors() )
	{
		CNodeAddress monitor;
		monitor.Set( allyMonitorData.m_publicKey.GetID(), common::NodePrefix::Monitor );
		info += "key " + monitor.ToString() + " ip " + allyMonitorData.m_address.ToString()
				+ ( CReputationTracker::getInstance()->isPresentNode( allyMonitorData.m_publicKey.GetID() ) ? "  active" : "  inactive" )+ "\n";
	}

	info += "Trackers: \n";
	BOOST_FOREACH( common::CAllyTrackerData const & allyTrackerData, CReputationTracker::getInstance()->getAllyTrackers() )
	{
		CNodeAddress tracker;
		tracker.Set( allyTrackerData.m_publicKey.GetID(), common::NodePrefix::Tracker );
		info += "key " + tracker.ToString() + " ip " + allyTrackerData.m_address.ToString()
				+ ( CReputationTracker::getInstance()->isPresentNode( allyTrackerData.m_publicKey.GetID() ) ? "  active" : "  inactive" )
				+ ( CReputationTracker::getInstance()->isTrackerSynchronized( allyTrackerData.m_publicKey.GetID() ) ? "  synchronized" : "  not synchronized" ) + "\n";
	}

	BOOST_FOREACH( common::CTrackerData const & trackerData, CReputationTracker::getInstance()->getTrackers() )
	{
		CNodeAddress tracker;
		tracker.Set( trackerData.m_publicKey.GetID(), common::NodePrefix::Tracker );
		info += "key " + tracker.ToString() + " ip " + trackerData.m_address.ToString()
				+ ( CReputationTracker::getInstance()->isPresentNode( trackerData.m_publicKey.GetID() ) ? "  active" : "  inactive" )
				+ ( CReputationTracker::getInstance()->isTrackerSynchronized( trackerData.m_publicKey.GetID() ) ? "  synchronized" : "  not synchronized" ) + "\n";
	}

	info +="\n\n\n bitcoin head block hash: " + chainActive.Tip()->GetBlockHash().ToString();

	if ( CTrackOriginAddressAction::getInstance() )
		info +="\nlast scaned block: \n\n" + CTrackOriginAddressAction::getInstance()->getCurrentHash().ToString();

	return info;
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

void registerHooks()
{
	SatusHook.connect( &getStatus );

	SelfAddress.connect( &selfAddress );

	EnterNetworkHook.connect( &enterNetwork );

	SynchronizeBitcoin.connect( &synchronizeBitcoin );

	NetworkInfo.connect( &networkInfo );

	SendCoins.connect( &sendCoins );
}

}
