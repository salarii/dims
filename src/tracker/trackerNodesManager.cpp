// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackerNodesManager.h"
#include "trackerNodeMedium.h"
#include "trackerMediumsKinds.h"
#include "connectNodeAction.h"

namespace common
{
std::vector< uint256 > deleteList;

template<>	CNodesManager< tracker::TrackerResponses > * common::CNodesManager< tracker::TrackerResponses >::ms_instance = 0;
}

namespace tracker
{

CTrackerNodesManager*
CTrackerNodesManager::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CTrackerNodesManager();
	};
	return static_cast<CTrackerNodesManager *>( ms_instance );
}


CTrackerNodesManager::CTrackerNodesManager()
{
	vector<CAddress> vAdd;

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

CTrackerNodeMedium*
CTrackerNodesManager::getMediumForNode( common::CSelfNode * _node ) const
{
	return static_cast< CTrackerNodeMedium* >( common::CNodesManager< TrackerResponses >::getMediumForNode( _node ) );
}

}
