// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_NODES_MANAGER_H
#define TRACKER_NODES_MANAGER_H

#include "common/nodesManager.h"
#include "common/communicationProtocol.h"
#include "common/connectionProvider.h"

#include "key.h"

class CNode;

namespace tracker
{
class CTrackerNodeMedium;

class CTrackerNodesManager : public common::CNodesManager
{
public:
	bool isNodeHonest();

	bool isBanned( CAddress const & _address ); // address may be banned  when , associated  node  make   trouble

	static CTrackerNodesManager * getInstance();

	std::set< common::CValidNodeInfo > const getNodesInfo( common::CRole::Enum _role ) const;

	bool getNodeInfo( uint160 const & _id, common::CValidNodeInfo & _validNodeInfo ) const;

	void setNodeInfo( common::CValidNodeInfo const & _validNodeInfo, common::CRole::Enum _role );

	std::list< common::CMedium *> getNodesByClass( common::CMediumKinds::Enum _nodesClass ) const;

	void setKeyToNode( CPubKey const & _pubKey, uintptr_t _nodeIndicator);

	bool getKeyToNode( CPubKey const & _pubKey, uintptr_t & _nodeIndicator)const;

	bool getKeyToNode( CKeyID const & _keyId, uintptr_t & _nodeIndicator)const;

	void eraseMedium( uintptr_t _nodePtr );

	unsigned int getNumberOfNetworkTrackers() const
	{
		return m_networkTrackers.size();
	}

	bool isInNetwork( uint160 const & _keyId )const;

	bool setNetworkTracker( common::CValidNodeInfo const & _nodeInfo );

	bool setNetworkMonitor( common::CValidNodeInfo const & _nodeInfo );

	void eraseNetworkTracker( uint160 const & _keyId );

	void eraseNetworkMonitor( uint160 const & _keyId );

	std::set< common::CValidNodeInfo > getNetworkTrackers()const;

	std::set< common::CValidNodeInfo > getNetworkMonitors()const;

	std::list< common::CMedium *> provideConnection( common::CMediumFilter const & _mediumFilter );

	void setActiveNode( uint160 const & _idKey );

	void removeActiveNode( uint160 const & _idKey );

	bool isActiveNode( uint160 const & _idKey ) const;

	bool getAddresFromKey( uint160 const & _pubKeyId, CAddress & _address )const;
private:
	std::set< common::CValidNodeInfo > extractValidNodeInfo( std::map< uint160, common::CValidNodeInfo > const & _validNode ) const;

	CTrackerNodesManager();
private:
	mutable boost::mutex m_lock;

	std::map< CKeyID, uintptr_t > m_pubKeyToNodeIndicator;

	std::map< uint160, common::CValidNodeInfo > m_trackers;

	std::map< uint160, common::CValidNodeInfo > m_monitors;

	std::map< uint160, common::CValidNodeInfo > m_seeds;

	std::map< uint160, common::CValidNodeInfo > m_networkTrackers;

	std::map< uint160, common::CValidNodeInfo > m_networkMonitors;

	std::set< uint160 > m_activeNodes;
};

}

#endif
