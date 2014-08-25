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
#include "trackerEvents.h"

namespace tracker
{


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
		m_synchronize = true;
	}
	else
	{
		common::CManageNetwork::getInstance()->getSeedIps( vAdd );

		// let know seed about our existence
		BOOST_FOREACH( CAddress address, vAdd )
		{
			common::CActionHandler< TrackerResponses >::getInstance()->executeAction( new CConnectNodeAction( address ) );
		}
		m_synchronize = false;
	}
}


boost::statechart::result
CStandAlone::react( CTrackerConnectedEvent const & _event )
{
	return m_synchronize ? transit< CSynchronizing >() : transit< CConnected >();// do it  after  synchronization  finishes ????
}



CSynchronizing::CSynchronizing( my_context ctx ) : my_base( ctx )
{
	CSynchronizationAction * synchronizationAction = new CSynchronizationAction();

	common::CActionHandler< TrackerResponses >::getInstance()->executeAction( synchronizationAction );
	synchronizationAction->process_event( CSwitchToSynchronizing() );

}


CConnected::CConnected( my_context ctx ) : my_base( ctx )
{
		CTrackerConnectedEvent const* trackerConnectedEvent
				= dynamic_cast< CTrackerConnectedEvent const* >( simple_state::triggering_event() );
}


}
