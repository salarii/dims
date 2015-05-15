// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "boost/foreach.hpp"

#include "protocol.h"
#include "base58.h"

#include "common/manageNetwork.h"
#include "common/actionHandler.h"

#include "tracker/connectNodeAction.h"
#include "tracker/synchronizationAction.h"
#include "tracker/trackerEvents.h"
#include "tracker/trackerController.h"
#include "tracker/trackOriginAddressAction.h"
#include "tracker/trackerControllerStates.h"
#include "tracker/recognizeNetworkAction.h"

namespace tracker
{

CInitialSynchronization::CInitialSynchronization()
	:m_blockLeft( -1 )
{
	common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( new tracker::CTrackOriginAddressAction );
}

boost::statechart::result
CInitialSynchronization::react( common::CSetScanBitcoinChainProgress const & _event )
{
	m_blockLeft = _event.m_blockLeft;
	return discard_event();
}

boost::statechart::result
CInitialSynchronization::react( common::CBitcoinNetworkConnection const & _event )
{
	m_nodesNumber = _event.m_nodesNumber;
	return discard_event();
}

boost::statechart::result
CInitialSynchronization::react( common::CUpdateStatus const & _event )
{
	std::string status;

	if ( m_nodesNumber < common::dimsParams().getUsedBitcoinNodesNumber() )
	{
		{
			std::ostringstream convert;
			convert << m_nodesNumber;

			status = "Connecting to bitcoin network \n currently there is:  " + convert.str() + "  connections \n";
		}
		{
			std::ostringstream convert;
			convert << common::dimsParams().getUsedBitcoinNodesNumber();

			status += "there is " + convert.str() + " needed";
		}
	}
	else
	{
		status = "Reading bitcoin blockchain for base transactions: \n";

		std::ostringstream convert;
		convert << m_blockLeft;

		status += "left " + convert.str() + " blocks to  be checked..\n";
	}
	context< CTrackerController >().setStatusMessage( status );
	return discard_event();
}

CStandAlone::CStandAlone( my_context ctx ) : my_base( ctx )
{
	common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( new CRecognizeNetworkAction() );
}

boost::statechart::result
CStandAlone::react( common::CNetworkRecognizedEvent const & _event )
{
	std::string status;

	status = "\nDetected following trackers \n";

	BOOST_FOREACH( common::CValidNodeInfo const & nodeInfo, _event.m_trackersInfo )
	{
		CNodeAddress tracker;
		tracker.Set( nodeInfo.m_key.GetID(), common::NodePrefix::Tracker );
		status = "key " + tracker.ToString() + "ip " + nodeInfo.m_address.ToString() + "\n";
	}

	status = "\nDetected following monitors \n";
	BOOST_FOREACH( common::CValidNodeInfo const & nodeInfo, _event.m_monitorsInfo )
	{
		CNodeAddress monitor;
		monitor.Set( nodeInfo.m_key.GetID(), common::NodePrefix::Monitor );
		status = "key " + monitor.ToString() + "ip " + nodeInfo.m_address.ToString() + "\n";
	}

	context< CTrackerController >().setStatusMessage( status );
	return discard_event();
}

CSynchronizing::CSynchronizing( my_context ctx ) : my_base( ctx )
{
	CSynchronizationAction * synchronizationAction = new CSynchronizationAction();

	common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( synchronizationAction );
	synchronizationAction->process_event( CSwitchToSynchronizing() );

}


CConnected::CConnected( my_context ctx ) : my_base( ctx )
{
	CTrackerController::getInstance()->setConnected( true );
}


}
