// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackerControllerStates.h"
#include "boost/foreach.hpp"
#include "protocol.h"

#include "common/manageNetwork.h"
#include "common/actionHandler.h"
#include "connectNodeAction.h"
#include "synchronizationAction.h"
#include "synchronizationEvents.h"

namespace tracker
{


CInitialSynchronization::CInitialSynchronization()
{
}

CStandAlone::CStandAlone( my_context ctx ) : my_base( ctx )
{
	// search for  seeder  action
	std::vector<CAddress> vAdd;

	common::CManageNetwork::getInstance()->getIpsFromSeed( vAdd );

	if ( !vAdd.empty() )
	{
		BOOST_FOREACH( CAddress address, vAdd )
		{
			common::CActionHandler< TrackerResponses >::getInstance()->executeAction( new CConnectNodeAction( address ) );
		}
	}
	else
	{
		common::CManageNetwork::getInstance()->getSeedIps( vAdd );

		// let know seed about our existence
		BOOST_FOREACH( CAddress address, vAdd )
		{
			common::CActionHandler< TrackerResponses >::getInstance()->executeAction( new CConnectNodeAction( address ) );
		}
	}
}

boost::statechart::result
CSynchronizing::react( CTrackerConnectedEvent const & _event )
{
	CSynchronizationAction * synchronizationAction = new CSynchronizationAction();

	common::CActionHandler< TrackerResponses >::getInstance()->executeAction( synchronizationAction );
	synchronizationAction->process_event( CSwitchToSynchronizing() );

	return transit< CConnected >();// do it  after  synchronization  finishes ????
}

boost::statechart::result
CStandAlone::react( CGetStateEvent const & _event )
{
	return discard_event();
}

CConnected::CConnected( my_context ctx ) : my_base( ctx )
{
		CTrackerConnectedEvent const* trackerConnectedEvent
				= dynamic_cast< CTrackerConnectedEvent const* >( simple_state::triggering_event() );
}


}
