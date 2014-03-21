// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

/*
This is  obvious crap 
network managment  should be managed in  the way:

		base managment
		/				\
ratcoin managment     bitcoin management


right  now  it is
net.h net.ccp  -  bitcoin  network

and 

this ( which is in most part  copy  paste from  above ) for  ratcoin  management


this  is ultra  ugly  and it has  to change, but  since  I have  more important  things to  do , I let it be

*/


#ifndef MANAGE_NETWORK_H
#define MANAGE_NETWORK_H 

class CManageNetwork
{
public:
	CManageNetwork();

	void startClientServer();

	mainLoop();

	/*
	validate  buffer 
	
	*/
private:
// some  of  this  is  copy paste from  net.cpp but it have to serve right now   

	void connectToNetwork();
	void negotiateWithMonitor();// if  at  all, not here

	void discover(boost::thread_group& threadGroup);

	bool addLocal(const CService& addr, int nScore);

	bool addLocal(const CNetAddr &addr, int nScore);

	bool OpenNetworkConnection(const CAddress& addrConnect, CSemaphoreGrant *grantOutbound, const char *strDest, bool fOneShot);

	void advertizeLocal();

	void threadSocketHandler();

	void threadOpenConnections();

	void threadMessageHandler();
private:

	static CSemaphore *ms_semOutbound;

	static unsigned int m_maxConnections;

	CNetworkParams * m_networkParams;

	Poco::Net::TCPServer  * m_tcpServer;

	CSelfNode * m_nodeLocalHost;

	CSelfNodesManager * m_nodesManager;

	CCriticalSection cs_vNodes;

	vector<CNode*> m_nodes;

	static list<CNode*> m_nodesDisconnected;

	static std::vector<SOCKET> m_listenSocket;

	CSemaphore *m_semOutbound;

	 CAddrMan addrman;
};


#endif // MANAGE_NETWORK_H