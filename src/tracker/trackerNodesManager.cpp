// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/mediumKinds.h"
#include "common/actionHandler.h"
#include "common/authenticationProvider.h"

#include "tracker/trackerNodesManager.h"
#include "tracker/trackerNodeMedium.h"
#include "tracker/connectNodeAction.h"
#include "tracker/controller.h"
#include "tracker/activityControllerAction.h"
#include "tracker/pingAction.h"

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
		return extractValidNodeInfo( m_seeds );
		break;
	case common::CRole::Tracker:
		return extractValidNodeInfo( m_trackers );
		break;
	case common::CRole::Monitor:
		return extractValidNodeInfo( m_monitors );
		break;
	default:
		return std::set< common::CValidNodeInfo >();
	}
}

bool
CTrackerNodesManager::getNodeInfo( uint160 const & _id, common::CValidNodeInfo & _validNodeInfo ) const
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	std::map< uint160, common::CValidNodeInfo >::const_iterator iterator = m_trackers.find( _id );

	if ( iterator != m_trackers.end() )
	{
		_validNodeInfo = iterator->second;
		return true;
	}

	iterator = m_monitors.find( _id );

	if ( iterator != m_monitors.end() )
	{
		_validNodeInfo = iterator->second;
		return true;
	}

	return false;
}


void
CTrackerNodesManager::setNodeInfo( common::CValidNodeInfo const & _validNodeInfo, common::CRole::Enum _role )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	switch( _role )
	{
	case common::CRole::Seed:
		m_seeds.insert( std::make_pair( _validNodeInfo.m_publicKey.GetID(), _validNodeInfo ) );
		break;
	case common::CRole::Tracker:
		// avoid  adding self
		if ( !(common::CAuthenticationProvider::getInstance()->getMyKey() == _validNodeInfo.m_publicKey) )
			m_trackers.insert( std::make_pair( _validNodeInfo.m_publicKey.GetID(), _validNodeInfo ) );
		break;
	case common::CRole::Monitor:
		m_monitors.insert( std::make_pair( _validNodeInfo.m_publicKey.GetID(), _validNodeInfo ) );
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
			if ( CController::getInstance()->isConnected() )
			{
				BOOST_FOREACH( PAIRTYPE( uint160, common::CValidNodeInfo ) const & validNode, m_networkTrackers )
				{
					if ( m_activeNodes.find( validNode.second.m_publicKey.GetID() ) != m_activeNodes.end() )
					{
						if (!getKeyToNode( validNode.second.m_publicKey, nodeIndicator ) )
							assert(!"something went wrong");

						common::CMedium * medium = findNodeMedium( nodeIndicator );
						if (!medium)
							assert(!"something went wrong");

						mediums.push_back( medium );
					}
				}
			}
			else
			{
				BOOST_FOREACH( PAIRTYPE( uint160, common::CValidNodeInfo ) const & validNode, m_trackers )
				{
					if (!getKeyToNode( validNode.second.m_publicKey, nodeIndicator ) )
						assert(!"something went wrong");

					common::CMedium * medium = findNodeMedium( nodeIndicator );
					if (!medium)
						assert(!"something went wrong");

					mediums.push_back( medium );
				}
			}
		}
		if ( _nodesClass != common::CMediumKinds::Trackers )
		{
			if ( CController::getInstance()->isConnected() )
			{
				BOOST_FOREACH( PAIRTYPE( uint160, common::CValidNodeInfo ) const & validNode, m_networkMonitors )
				{
					if ( m_activeNodes.find( validNode.second.m_publicKey.GetID() ) != m_activeNodes.end() )
					{
						if (!getKeyToNode( validNode.second.m_publicKey, nodeIndicator ) )
							assert(!"something went wrong");

						common::CMedium * medium = findNodeMedium( nodeIndicator );
						if (!medium)
							assert(!"something went wrong");

						mediums.push_back( medium );
					}
				}
			}
			else
			{
				BOOST_FOREACH( PAIRTYPE( uint160, common::CValidNodeInfo ) const & validNode, m_monitors )
				{

					if (!getKeyToNode( validNode.second.m_publicKey, nodeIndicator ) )
						assert(!"something went wrong");

					common::CMedium * medium = findNodeMedium( nodeIndicator );
					if (!medium)
						assert(!"something went wrong");

					mediums.push_back( medium );
				}
			}
		}
	}
	else if ( _nodesClass == common::CMediumKinds::Seeds )
	{
		BOOST_FOREACH( PAIRTYPE( uint160, common::CValidNodeInfo ) const & validNode, m_seeds )
		{
			if (!getKeyToNode( validNode.second.m_publicKey, nodeIndicator ) )
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
	CPubKey pubKey;
	if ( !getAddress( _nodePtr, address ) )
		return;

	common::CNodesManager::eraseMedium( _nodePtr );

	if ( !getPublicKey( address, pubKey ) )
		return;

	if ( m_pubKeyToNodeIndicator.find( pubKey.GetID() ) != m_pubKeyToNodeIndicator.end() )
		m_pubKeyToNodeIndicator.erase( pubKey.GetID() );

	m_activeNodes.erase( pubKey.GetID() );

	erasePubKey( address );

	common::CActionHandler::getInstance()->executeAction( new CActivityControllerAction( pubKey, address, CActivitySatatus::Inactive ) );
}
bool
CTrackerNodesManager::isInNetwork( uint160 const & _keyId )const
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	return
			m_networkTrackers.find( _keyId ) != m_networkTrackers.end()
			|| m_networkMonitors.find( _keyId ) != m_networkMonitors.end();
}

void
CTrackerNodesManager::evaluateNode( common::CSelfNode * _selfNode )
{
	CPubKey pubKey;
	if ( getPublicKey( _selfNode->addr, pubKey ) )
	{
		common::CActionHandler::getInstance()->executeAction( new CPingAction( _selfNode ) );
	}
	else
	{
		_selfNode->fDisconnect = true;
	}
}

bool
CTrackerNodesManager::setNetworkTracker( common::CValidNodeInfo const & _nodeInfo )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	if ( !(common::CAuthenticationProvider::getInstance()->getMyKey() == _nodeInfo.m_publicKey) )
	{
		m_networkTrackers.insert( std::make_pair( _nodeInfo.m_publicKey.GetID(), _nodeInfo ) );
		m_trackers.insert( std::make_pair( _nodeInfo.m_publicKey.GetID(), _nodeInfo ) );
	}
}

bool
CTrackerNodesManager::setNetworkMonitor( common::CValidNodeInfo const & _nodeInfo )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	m_networkMonitors.insert( std::make_pair( _nodeInfo.m_publicKey.GetID(), _nodeInfo ) );
	m_monitors.insert( std::make_pair( _nodeInfo.m_publicKey.GetID(), _nodeInfo ) );
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

	return extractValidNodeInfo( m_networkTrackers );
}

std::set< common::CValidNodeInfo >
CTrackerNodesManager::getNetworkMonitors()const
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	return extractValidNodeInfo( m_networkMonitors );
}

std::set< common::CValidNodeInfo >
CTrackerNodesManager::extractValidNodeInfo( std::map< uint160, common::CValidNodeInfo > const & _validNode ) const
{
	std::set< common::CValidNodeInfo > nodes;

	BOOST_FOREACH( PAIRTYPE( uint160, common::CValidNodeInfo ) const & nodeInfo, _validNode )
	{
		nodes.insert( nodeInfo.second );
	}

	return nodes;
}

std::list< common::CMedium *>
CTrackerNodesManager::provideConnection( common::CMediumFilter const & _mediumFilter )
{
	std::list< common::CMedium*> mediums = common::CNodesManager::provideConnection( _mediumFilter );

	if ( !mediums.empty() )
		return mediums;

	return _mediumFilter.getMediums( this );
}

void
CTrackerNodesManager::setActiveNode( uint160 const & _idKey )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	m_activeNodes.insert( _idKey );
}

void
CTrackerNodesManager::removeActiveNode( uint160 const & _idKey )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	m_activeNodes.erase( _idKey );
}

bool
CTrackerNodesManager::isActiveNode( uint160 const & _idKey ) const
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	return m_activeNodes.find( _idKey ) != m_activeNodes.end();
}

bool
CTrackerNodesManager::getAddresFromKey( uint160 const & _pubKeyId, CAddress & _address )const
{
	uintptr_t nodeIndicator;
	if ( !getKeyToNode( _pubKeyId, nodeIndicator ) )
		return false;

	return getAddress( nodeIndicator, _address );
}

}
