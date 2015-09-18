// Copyright (c) 2014-2015 Dims dev-team
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

	void setNodeInfo( common::CValidNodeInfo const & _validNodeInfo, common::CRole::Enum _role );

	void setPublicKey( CAddress const & _address, CPubKey const & _pubKey );

	bool getPublicKey( CAddress const & _address, CPubKey & _pubKey ) const;

	std::list< common::CMedium *> getNodesByClass( common::CMediumKinds::Enum _nodesClass ) const;

	void setKeyToNode( CPubKey const & _pubKey, uintptr_t _nodeIndicator);

	bool getKeyToNode( CPubKey const & _pubKey, uintptr_t & _nodeIndicator)const;

	bool getKeyToNode( CKeyID const & _keyId, uintptr_t & _nodeIndicator)const;

	void eraseMedium( uintptr_t _nodePtr );

	unsigned int getNumberOfTrackers() const
	{
		return m_trackers.size();
	}
private:
	CTrackerNodesManager();
private:
	// is this ok??? seems like temporary solution, move it  to  common???
	std::map< CAddress, CPubKey > m_keyStore;

	std::map< CKeyID, uintptr_t > m_pubKeyToNodeIndicator;

	std::set< common::CValidNodeInfo > m_trackers;

	std::set< common::CValidNodeInfo > m_monitors;

	std::set< common::CValidNodeInfo > m_seeds;
};

}

#endif
