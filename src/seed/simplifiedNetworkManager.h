// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SIMPLIFIED_NETWORK_MANAGER_H
#define SIMPLIFIED_NETWORK_MANAGER_H


#include <list>
#include <vector>

#include <boost/thread.hpp>
#include <boost/signals2.hpp>
#include "netbase.h"


class CSimplifiedNetworkManager
{
public:
	CSimplifiedNetworkManager();

	bool connectToNetwork( boost::thread_group& threadGroup );

/*	struct LocalServiceInfo
	{
		int nScore;
		int nPort;
	};
	typedef int NodeId;
	struct CNodeSignals
	{
		boost::signals2::signal<bool (CSelfNode*)> ProcessMessages;
		boost::signals2::signal<bool (CSelfNode*, bool)> SendMessages;
		boost::signals2::signal<void (NodeId, const CSelfNode*)> InitializeNode;
		boost::signals2::signal<bool (CSelfNode*, CDataStream&)> ProcessMessage;
		boost::signals2::signal<void (NodeId)> FinalizeNode;
	};

	enum BindFlags {
		BF_NONE         = 0,
		BF_EXPLICIT     = (1U << 0),
		BF_REPORT_ERROR = (1U << 1)
	};
public:
	void mainLoop();

	CSelfNode* connectNode(CAddress addrConnect, const char *pszDest);

	static CManageNetwork* getInstance();

	template < class Handler >
	void registerNodeSignals( Handler * _handler );
private:

// some  of  this  is  copy paste from  net.cpp

	void negotiateWithMonitor();// if  at  all, not here

	void discover(boost::thread_group& threadGroup);

	bool addLocal(const CService& addr, int nScore);

	bool addLocal(const CNetAddr &addr, int nScore);

	bool
	openNetworkConnection(const CAddress& addrConnect, CSemaphoreGrant *grantOutbound, const char *strDest, bool fOneShot = false);

	CSelfNode* findNode(const CNetAddr& ip);

	bool bindListenPort(const CService &addrBind, string& strError);

	bool bind(const CService &addr, unsigned int flags);

	CSelfNode* findNode(std::string addrName);

	CSelfNode* findNode(const CService& addr);

	void advertizeLocal();

	void threadSocketHandler();

	void threadOpenConnections();

	void threadMessageHandler();

	void threadOpenAddedConnections();

	void StartSync(const vector<CSelfNode*> &vNodes);

	bool processMessages(CSelfNode* pfrom);

	void processGetData(CSelfNode* pfrom);

	template < class Handler >
	void unregisterNodeSignals( Handler * _handler );
private:
	static CManageNetwork * ms_instance;

	CNetworkParams * m_networkParams;

	CSelfNode * m_nodeLocalHost;

	CCriticalSection cs_vNodes;
	CCriticalSection cs_mapLocalHost;
	CCriticalSection cs_setservAddNodeAddresses;

	set<CNetAddr> setservAddNodeAddresses;
	std::vector<CSelfNode*> m_nodes;

	std::list<CSelfNode*> m_nodesDisconnected;

	std::vector<SOCKET> m_listenSocket;

	map<CNetAddr, LocalServiceInfo> mapLocalHost;

	CSelfNode* pnodeSync;
	CAddrMan addrman;
	CNodeSignals m_signals;*/
	uint64_t nLocalServices;

	unsigned int m_maxConnections;
};


#endif // SIMPLIFIED_NETWORK_MANAGER_H
