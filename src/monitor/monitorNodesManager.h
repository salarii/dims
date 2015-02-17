// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MONITOR_NODES_MANAGER_H
#define MONITOR_NODES_MANAGER_H


#include "common/communicationProtocol.h"
#include "common/connectionProvider.h"
#include "configureMonitorActionHandler.h"
#include "key.h"

class CNode;

namespace monitor
{

class CMonitorNodeMedium;

class CMonitorNodesManager
{
public:
	static CMonitorNodesManager * getInstance();
private:
	CMonitorNodesManager();
private:
	// locks  	mutable boost::mutex m_nodesLock;


	//valid network store it  is temporary  solution, move it  to  common???

};

}

#endif // MONITOR_NODES_MANAGER_H
