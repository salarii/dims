// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "monitorNodesManager.h"
#include "monitorNodeMedium.h"
#include "connectNodeAction.h"

#include "common/mediumKinds.h"

namespace common
{

template<>	CNodesManager< monitor::MonitorResponses > * common::CNodesManager< monitor::MonitorResponses >::ms_instance = 0;
}

namespace monitor
{

CMonitorNodesManager*
CMonitorNodesManager::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CMonitorNodesManager();
	};
	return static_cast<CMonitorNodesManager *>( ms_instance );
}


CMonitorNodesManager::CMonitorNodesManager()
{
}

CMonitorNodeMedium*
CMonitorNodesManager::getMediumForNode( common::CSelfNode * _node ) const
{
	return static_cast< CMonitorNodeMedium* >( common::CNodesManager< MonitorResponses >::getMediumForNode( _node ) );
}

}
