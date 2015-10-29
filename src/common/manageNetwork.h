// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MANAGE_NETWORK_H
#define MANAGE_NETWORK_H 

#include <list>
#include <vector>

#include <boost/thread.hpp>
#include <boost/signals2.hpp>
#include "netbase.h"
#include "addrman.h"

class CSelfNode;

namespace common
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

	bool connectToNetwork( boost::thread_group& threadGroup );

	template < class Handler >
	void registerNodeSignals( Handler * _handler );

	void getIpsFromSeed( vector<CAddress> & _vAdd );

	void getSeedIps( vector<CAddress> & _vAdd );
private:
	CManageNetwork();

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

	CSemaphore *m_semOutbound;

	unsigned int m_maxConnections;

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

	uint64_t nLocalServices;
	CSelfNode* pnodeSync;
	CAddrMan addrman;
	CNodeSignals m_signals;
};

template < class Handler >
void
CManageNetwork::registerNodeSignals( Handler * _handler )
{
	m_signals.ProcessMessages.connect(boost::bind( &CManageNetwork::processMessages, this, _1 ));

	m_signals.SendMessages.connect(boost::bind( &Handler::sendMessages, _handler, _1, _2 ) );

	m_signals.ProcessMessage.connect(boost::bind( &Handler::processMessage, _handler, _1, _2 ));
//	m_signals.InitializeNode.connect(boost::bind( &CManageNetwork::initializeNode, this ));
//	m_signals.FinalizeNode.connect(boost::bind( &CManageNetwork::finalizeNode, this ));
}

template < class Handler >
void
CManageNetwork::unregisterNodeSignals( Handler * _handler )
{
	m_signals.ProcessMessages.disconnect(boost::bind( &CManageNetwork::processMessages, this, _1 ));

	m_signals.SendMessages.disconnect(boost::bind( &Handler::sendMessages, this, _1, _2 ) );

	m_signals.ProcessMessage.disconnect(boost::bind( &Handler::processMessage, _handler, _1, _2 ));
//	m_signals.InitializeNode.disconnect(boost::bind( &CManageNetwork::initializeNode, this ));
//	m_signals.FinalizeNode.disconnect(boost::bind( &CManageNetwork::finalizeNode, this ));
}


}

#endif // MANAGE_NETWORK_H
