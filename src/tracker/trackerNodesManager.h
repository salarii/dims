// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_NODES_MANAGER_H
#define TRACKER_NODES_MANAGER_H

#include "common/nodesManager.h"
#include "common/communicationProtocol.h"
#include "common/connectionProvider.h"
#include "configureTrackerActionHandler.h"
#include "key.h"

class CNode;

namespace tracker
{
class CTrackerNodeMedium;

class CTrackerNodesManager : public common::CNodesManager< TrackerResponses >
{
public:

	bool isNodeHonest();

	bool isBanned( CAddress const & _address ); // address may be banned  when , associated  node  make   trouble

	static CTrackerNodesManager * getInstance();

	CTrackerNodeMedium* getMediumForNode( common::CSelfNode * _node ) const;

	bool getKeyForNode( common::CSelfNode * _node, CPubKey & _key ) const;
private:
	CTrackerNodesManager();
private:
	// is this ok??? seems like temporary solution
	std::map< common::CSelfNode *, CPubKey > m_keyStore;
};

}

#endif
