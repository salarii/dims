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

	enum BindFlags {
		BF_NONE         = 0,
		BF_EXPLICIT     = (1U << 0),
		BF_REPORT_ERROR = (1U << 1)
	};
public:
	void mainLoop();

	/*
	validate  buffer 
	
	*/
	static CManageNetwork* getInstance();

	bool connectToNetwork( boost::thread_group& threadGroup );
private:
	CManageNetwork();
// some  of  this  is  copy paste from  net.cpp but it have to serve right now   

	void negotiateWithMonitor();// if  at  all, not here

	void discover(boost::thread_group& threadGroup);

	bool addLocal(const CService& addr, int nScore);

	bool addLocal(const CNetAddr &addr, int nScore);

	bool
	openNetworkConnection(const CAddress& addrConnect, CSemaphoreGrant *grantOutbound, const char *strDest, bool fOneShot = false);

	CNode* connectNode(CAddress addrConnect, const char *pszDest);

	CNode* findNode(const CNetAddr& ip);

	bool bindListenPort(const CService &addrBind, string& strError);

	bool bind(const CService &addr, unsigned int flags);

	CNode* findNode(std::string addrName);

	CNode* findNode(const CService& addr);

	void advertizeLocal();

	void threadSocketHandler();

	void threadOpenConnections();

	void threadMessageHandler();

	void threadOpenAddedConnections();

	void StartSync(const vector<CNode*> &vNodes);
private:
	static CManageNetwork * ms_instance;

	CSemaphore *m_semOutbound;

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

	uint64_t nLocalServices;
	CNode* pnodeSync;
	CAddrMan addrman;
	CNodeSignals g_signals;
};

}

#endif // MANAGE_NETWORK_H
