// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackerNodesManager.h"
#include "trackerNodeMedium.h"
#include "connectNodeAction.h"

#include "common/mediumKinds.h"

namespace common
{
std::vector< uint256 > deleteList;
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
	return dynamic_cast<CTrackerNodesManager *>( ms_instance );
}


CTrackerNodesManager::CTrackerNodesManager()
{
}

std::set< common::CValidNodeInfo > const
CTrackerNodesManager::getNodesInfo( common::CRole::Enum _role ) const
{
	boost::lock_guard<boost::mutex> lock( m_lock );
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
	boost::lock_guard<boost::mutex> lock( m_lock );

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


std::list< common::CMedium *>
CTrackerNodesManager::getNodesByClass( common::CMediumKinds::Enum _nodesClass ) const
{
	//  code  repeated  3 times ,fix it??

	uintptr_t nodeIndicator;
	std::list< common::CMedium *> mediums;

	if ( _nodesClass == common::CMediumKinds::DimsNodes || _nodesClass == common::CMediumKinds::Trackers || _nodesClass == common::CMediumKinds::Monitors )
	{
		if ( _nodesClass != common::CMediumKinds::Monitors )
		{
			BOOST_FOREACH( common::CValidNodeInfo const & validNode, m_trackers )
			{

				if (!getKeyToNode( validNode.m_publicKey, nodeIndicator ) )
					assert(!"something went wrong");

				common::CMedium * medium = findNodeMedium( nodeIndicator );
				if (!medium)
					assert(!"something went wrong");

				mediums.push_back( medium );
			}
		}
		if ( _nodesClass != common::CMediumKinds::Trackers )
		{
			BOOST_FOREACH( common::CValidNodeInfo const & validNode, m_monitors )
			{

				if (!getKeyToNode( validNode.m_publicKey, nodeIndicator ) )
					assert(!"something went wrong");

				common::CMedium * medium = findNodeMedium( nodeIndicator );
				if (!medium)
					assert(!"something went wrong");

				mediums.push_back( medium );
			}
		}
	}
	else if ( _nodesClass == common::CMediumKinds::Seeds )
	{
		BOOST_FOREACH( common::CValidNodeInfo const & validNode, m_seeds )
		{

			if (!getKeyToNode( validNode.m_publicKey, nodeIndicator ) )
				assert(!"something went wrong");

			common::CMedium * medium = findNodeMedium( nodeIndicator );
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
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_pubKeyToNodeIndicator.insert( std::make_pair( _pubKey.GetID(), _nodeIndicator ) );
}

bool
CTrackerNodesManager::getKeyToNode( CPubKey const & _pubKey, uintptr_t & _nodeIndicator) const
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	std::map< CKeyID, uintptr_t >::const_iterator iterator = m_pubKeyToNodeIndicator.find( _pubKey.GetID() );

	if ( iterator != m_pubKeyToNodeIndicator.end() )
		_nodeIndicator = iterator->second;

	return iterator != m_pubKeyToNodeIndicator.end();
}

bool
CTrackerNodesManager::getKeyToNode( CKeyID const & _keyId, uintptr_t & _nodeIndicator) const
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	std::map< CKeyID, uintptr_t >::const_iterator iterator = m_pubKeyToNodeIndicator.find( _keyId );

	if ( iterator != m_pubKeyToNodeIndicator.end() )
		_nodeIndicator = iterator->second;

	return iterator != m_pubKeyToNodeIndicator.end();
}

void
CTrackerNodesManager::eraseMedium( uintptr_t _nodePtr )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	CAddress address;

	getAddress( _nodePtr, address );

	CPubKey pubKey;

	getPublicKey( address, pubKey );

	m_keyStore.erase( address );

	m_pubKeyToNodeIndicator.erase( pubKey.GetID() );

	m_trackers.erase( common::CValidNodeInfo( pubKey, address ) );

	m_monitors.erase( common::CValidNodeInfo( pubKey, address ) );

	m_seeds.erase( common::CValidNodeInfo( pubKey, address ) );
}

bool
CTrackerNodesManager::isInNetwork( uint160 const & _keyId )const
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	return
			m_networkTrackers.find( _keyId ) != m_networkTrackers.end()
			|| m_networkMonitors.find( _keyId ) != m_networkMonitors.end();
}

bool
CTrackerNodesManager::setNetworkTracker( common::CValidNodeInfo const & _nodeInfo )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	m_networkTrackers.insert( std::make_pair( _nodeInfo.m_publicKey.GetID(), _nodeInfo ) );
}

bool
CTrackerNodesManager::setNetworkMonitor( common::CValidNodeInfo const & _nodeInfo )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	m_networkMonitors.insert( std::make_pair( _nodeInfo.m_publicKey.GetID(), _nodeInfo ) );
}

void
CTrackerNodesManager::eraseNetworkTracker( uint160 const & _keyId )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	m_networkTrackers.erase( _keyId );
}

void
CTrackerNodesManager::eraseNetworkMonitor( uint160 const & _keyId )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	m_networkMonitors.erase( _keyId );
}

std::set< common::CValidNodeInfo >
CTrackerNodesManager::getNetworkTrackers()const
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	std::set< common::CValidNodeInfo > trackers;

	BOOST_FOREACH( PAIRTYPE( uint160, common::CValidNodeInfo ) const & nodeInfo, m_networkTrackers )
	{
		trackers.insert( nodeInfo.second );
	}

	return trackers;
}

std::set< common::CValidNodeInfo >
CTrackerNodesManager::getNetworkMonitors()const
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	std::set< common::CValidNodeInfo > monitors;

	BOOST_FOREACH( PAIRTYPE( uint160, common::CValidNodeInfo ) const & nodeInfo, m_networkTrackers )
	{
		monitors.insert( nodeInfo.second );
	}

	return monitors;
}

}
