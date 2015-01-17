// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackerNodesManager.h"
#include "trackerNodeMedium.h"
#include "connectNodeAction.h"

#include "common/mediumKinds.h"

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
}

CTrackerNodeMedium*
CTrackerNodesManager::getMediumForNode( common::CSelfNode * _node ) const
{
	return static_cast< CTrackerNodeMedium* >( common::CNodesManager< TrackerResponses >::getMediumForNode( _node ) );
}

std::set< common::CValidNodeInfo > const &
CTrackerNodesManager::getValidNodes() const
{
	return m_validNodes;
}

void
CTrackerNodesManager::setValidNode( common::CValidNodeInfo const & _validNodeInfo )
{
	m_validNodes.insert( _validNodeInfo );
}


void
CTrackerNodesManager::setPublicKey( CAddress const & _address, CPubKey const & _pubKey )
{
	m_keyStore.insert( std::make_pair( _address, _pubKey ) );
}

bool
CTrackerNodesManager::getPublicKey( CAddress const & _address, CPubKey & _pubKey ) const
{
	std::map< CAddress, CPubKey >::const_iterator iterator = m_keyStore.find( _address );

	if ( iterator == m_keyStore.end() )
		return false;

	_pubKey = iterator->second;

	return true;
}

}
