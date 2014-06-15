#ifndef SEED_NODES_MANAGER_H
#define SEED_NODES_MANAGER_H

#include "common/nodesManager.h"
#include "common/communicationProtocol.h"
#include "common/connectionProvider.h"
#include "configureSeedActionHandler.h"

namespace seed
{

class CTrackerNodeMedium;

class CTrackerNodesManager : public common::CNodesManager< SeedResponses >
{
public:

	static CTrackerNodesManager * getInstance();

	CTrackerNodeMedium* getMediumForNode( common::CSelfNode * _node ) const;
private:
	CTrackerNodesManager();
private:
};

}

#endif // SEED_NODES_MANAGER_H
