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

template<> CNodesManager< common::CTrackerTypes > * common::CNodesManager< common::CTrackerTypes >::ms_instance = 0;
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

std::set< common::CValidNodeInfo > const
CTrackerNodesManager::getNodesInfo( common::CRole::Enum _role ) const
{
	switch( _role )
	{
	case common::CRole::Seed:
		return m_seeds;
		break;
	case common::CRole::Tracker:
		return m_trackers;
		break;
	case common::CRole::Monitor:
		return m_monitors;
		break;
	default:
		return std::set< common::CValidNodeInfo >();
	}
}

void
CTrackerNodesManager::setNodeInfo( common::CValidNodeInfo const & _validNodeInfo, common::CRole::Enum _role )
{
	switch( _role )
	{
	case common::CRole::Seed:
		m_seeds.insert( _validNodeInfo );
		break;
	case common::CRole::Tracker:
		m_trackers.insert( _validNodeInfo );
		break;
	case common::CRole::Monitor:
		m_monitors.insert( _validNodeInfo );
		break;
	default:
		break;
	}
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

std::list< common::CTrackerBaseMedium *>
CTrackerNodesManager::getNodesByClass( common::CMediumKinds::Enum _nodesClass ) const
{
	//  code  repeated  3 times ,fix it??

		uintptr_t nodeIndicator;
		std::list< common::CTrackerBaseMedium *> mediums;

	if ( common::CMediumKinds::DimsNodes || common::CMediumKinds::Trackers )
	{
		BOOST_FOREACH( common::CValidNodeInfo const & validNode, m_trackers )
		{

			if (!getKeyToNode( validNode.m_key, nodeIndicator ) )
				assert(!"something went wrong");

			common::CTrackerBaseMedium * medium = findNodeMedium( nodeIndicator );
			if (!medium)
				assert(!"something went wrong");

			mediums.push_back( medium );
		}
	}
	else if ( common::CMediumKinds::DimsNodes || common::CMediumKinds::Monitors )
	{
		BOOST_FOREACH( common::CValidNodeInfo const & validNode, m_monitors )
		{

			if (!getKeyToNode( validNode.m_key, nodeIndicator ) )
				assert(!"something went wrong");

			common::CTrackerBaseMedium * medium = findNodeMedium( nodeIndicator );
			if (!medium)
				assert(!"something went wrong");

			mediums.push_back( medium );
		}
	}
	else if ( common::CMediumKinds::Seeds )
	{
		BOOST_FOREACH( common::CValidNodeInfo const & validNode, m_seeds )
		{

			if (!getKeyToNode( validNode.m_key, nodeIndicator ) )
				assert(!"something went wrong");

			common::CTrackerBaseMedium * medium = findNodeMedium( nodeIndicator );
			if (!medium)
				assert(!"something went wrong");

			mediums.push_back( medium );
		}
	}

	return mediums;
}

void
CTrackerNodesManager::setKeyToNode( CPubKey const & _pubKey, uintptr_t _nodeIndicator)
{
	m_pubKeyToNodeIndicator.insert( std::make_pair( _pubKey, _nodeIndicator ) );
}

bool
CTrackerNodesManager::getKeyToNode( CPubKey const & _pubKey, uintptr_t & _nodeIndicator) const
{
	std::map< CPubKey, uintptr_t >::const_iterator iterator = m_pubKeyToNodeIndicator.find( _pubKey );

	if ( iterator != m_pubKeyToNodeIndicator.end() )
		_nodeIndicator = iterator->second;

	return iterator != m_pubKeyToNodeIndicator.end();
}

}
