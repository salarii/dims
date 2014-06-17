// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "seedNodesManager.h"
#include "seedNodeMedium.h"

namespace common
{
std::vector< uint256 > deleteList;

template<>	CNodesManager< seed::SeedResponses > * common::CNodesManager< seed::SeedResponses >::ms_instance = 0;
}

namespace seed
{

CSeedNodesManager*
CSeedNodesManager::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CSeedNodesManager();
	};
	return static_cast<CSeedNodesManager *>( ms_instance );
}


CSeedNodesManager::CSeedNodesManager()
{
}

CSeedNodeMedium*
CSeedNodesManager::getMediumForNode( common::CSelfNode * _node ) const
{
	return static_cast< CSeedNodeMedium* >( common::CNodesManager< SeedResponses >::getMediumForNode( _node ) );
}

}
