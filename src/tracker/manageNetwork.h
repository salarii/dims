// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/*
Redesign it this way:
		base managment
		/				\
ratcoin managment     bitcoin management

*/


#ifndef MANAGE_NETWORK_H
#define MANAGE_NETWORK_H 

#include <list>
#include <vector>

#include <boost/thread.hpp>
#include <boost/signals2.hpp>
#include "netbase.h"
#include "addrman.h"

class CNode;

namespace tracker
{

class CSelfNode;

class CSelfNodesManager;

class CManageNetwork
{
public:
	struct LocalServiceInfo
	{
		int nScore;
		int nPort;
	};
	typedef int NodeId;
	struct CNodeSignals
	{
		boost::signals2::signal<int ()> GetHeight;
		boost::signals2::signal<bool (CNode*)> ProcessMessages;
		boost::signals2::signal<bool (CNode*, bool)> SendMessages;
		boost::signals2::signal<void (NodeId, const CNode*)> InitializeNode;
		boost::signals2::signal<void (NodeId)> FinalizeNode;
	};
public:
	CManageNetwork();

	void mainLoop();

	/*
	validate  buffer 
	
	*/
private:
// some  of  this  is  copy paste from  net.cpp but it have to serve right now   

	void connectToNetwork( boost::thread_group& threadGroup );
	void negotiateWithMonitor();// if  at  all, not here

	void discover(boost::thread_group& threadGroup);

	bool addLocal(const CService& addr, int nScore);

	bool addLocal(const CNetAddr &addr, int nScore);

	bool
	openNetworkConnection(const CAddress& addrConnect, CSemaphoreGrant *grantOutbound, const char *strDest, bool fOneShot);

	void advertizeLocal();

	void threadSocketHandler();

	void threadOpenConnections();

	void threadMessageHandler();

	void threadOpenAddedConnections();

	void StartSync(const vector<CNode*> &vNodes);
private:

	CSemaphore *ms_semOutbound;

	unsigned int m_maxConnections;

	CNetworkParams * m_networkParams;

	CSelfNode * m_nodeLocalHost;

	CSelfNodesManager * m_nodesManager;

	CCriticalSection cs_vNodes;
	CCriticalSection cs_mapLocalHost;
	CCriticalSection cs_setservAddNodeAddresses;

	set<CNetAddr> setservAddNodeAddresses;
	std::vector<CNode*> m_nodes;

	std::list<CNode*> m_nodesDisconnected;

	std::vector<SOCKET> m_listenSocket;

	map<CNetAddr, LocalServiceInfo> mapLocalHost;

	CSemaphore *m_semOutbound;

	uint64_t nLocalServices;
	CNode* pnodeSync;
	CAddrMan addrman;
	CNodeSignals g_signals;
};

}

#endif // MANAGE_NETWORK_H
