// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "manageNetwork.h"
#include "sync.h"
#include "protocol.h"
#include "selfNode.h"
#include "main.h"

#include "common/ratcoinParams.h"

#include "communicationProtocol.h"
#include "nodesManager.h"

static const int MAX_OUTBOUND_CONNECTIONS = 64;

namespace tracker
{

CManageNetwork * CManageNetwork::ms_instance = NULL;

CManageNetwork*
CManageNetwork::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CManageNetwork();
	};
	return ms_instance;
}

CManageNetwork::CManageNetwork()
	: m_semOutbound( NULL )
	, m_maxConnections( 16 )
	, nLocalServices( NODE_NETWORK )
	, pnodeSync( NULL )
{
}
/*
void 
CManageNetwork::startClientServer()
{
	Poco::Net::ServerSocket serverSocket( m_networkParams->GetDefaultPort() );

	//Configure some server params.
	Poco::Net::TCPServerParams* pParams = new Poco::Net::TCPServerParams();
	pParams->setMaxThreads(4);
	pParams->setMaxQueued(10);
	pParams->setThreadIdleTime(50);

	//Create your server
	m_tcpServer = new Poco::Net::TCPServer(new Poco::Net::TCPServerConnectionFactoryImpl<newConnection>(), serverSocket, pParams);
	m_tcpServer->start();
}
*/
bool
CManageNetwork::connectToNetwork( boost::thread_group& threadGroup )
{
	bool bound = false;
	struct in_addr inaddr_any;
	inaddr_any.s_addr = INADDR_ANY;

#ifdef USE_IPV6
	bound = bind(CService(in6addr_any, GetListenPort< common::CRatcoinParams >()), BF_NONE);
#endif
	bound = bind(CService(inaddr_any, GetListenPort< common::CRatcoinParams >()), BF_REPORT_ERROR );

	if (m_semOutbound == NULL) {
		// initialize semaphore
		m_semOutbound = new CSemaphore(m_maxConnections);
	}

	if (m_nodeLocalHost == NULL)
		m_nodeLocalHost = new CSelfNode(INVALID_SOCKET, CAddress(CService("127.0.0.1", 0), nLocalServices));

	discover(threadGroup);

	//
	// Start threads
	//
/* not  seeding right now
	if (!GetBoolArg("-dnsseed", true))
		LogPrintf("DNS seeding disabled\n");
	else
		threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "dnsseed", &ThreadDNSAddressSeed));
*/
/*  disable  this for the time being 
#ifdef USE_UPNP
	// Map ports with UPnP
	MapPort(GetBoolArg("-upnp", USE_UPNP));
#endif
*/
	// Send and receive from sockets, accept connections

	threadGroup.create_thread(boost::bind(&tracker::CManageNetwork::threadSocketHandler, this));

	// Initiate outbound connections from -addnode
//	threadGroup.create_thread(boost::bind(&tracker::CManageNetwork::threadOpenAddedConnections, this));

	// Initiate outbound connections
	threadGroup.create_thread(boost::bind(&tracker::CManageNetwork::threadOpenConnections, this));

	// Process messages
	threadGroup.create_thread(boost::bind(&tracker::CManageNetwork::threadMessageHandler, this));

	// Dump network addresses
//	threadGroup.create_thread(boost::bind(&LoopForever<void (*)()>, "dumpaddr", &DumpAddresses, DUMP_ADDRESSES_INTERVAL * 1000));

	return bound;
}

void
CManageNetwork::discover(boost::thread_group& threadGroup)
{
	if (!fDiscover)
		return;

#ifdef WIN32
	// Get local host IP
	char pszHostName[1000] = "";
	if (gethostname(pszHostName, sizeof(pszHostName)) != SOCKET_ERROR)
	{
		vector<CNetAddr> vaddr;
		if (LookupHost(pszHostName, vaddr))
		{
			BOOST_FOREACH (const CNetAddr &addr, vaddr)
			{
				AddLocal(addr, LOCAL_IF);
			}
		}
	}
#else
	// Get local host ip
	struct ifaddrs* myaddrs;
	if (getifaddrs(&myaddrs) == 0)
	{
		for (struct ifaddrs* ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
		{
			if (ifa->ifa_addr == NULL) continue;
			if ((ifa->ifa_flags & IFF_UP) == 0) continue;
			if (strcmp(ifa->ifa_name, "lo") == 0) continue;
			if (strcmp(ifa->ifa_name, "lo0") == 0) continue;
			if (ifa->ifa_addr->sa_family == AF_INET)
			{
				struct sockaddr_in* s4 = (struct sockaddr_in*)(ifa->ifa_addr);
				CNetAddr addr(s4->sin_addr);
				if (AddLocal(addr, LOCAL_IF))
					LogPrintf("IPv4 %s: %s\n", ifa->ifa_name, addr.ToString());
			}
#ifdef USE_IPV6
			else if (ifa->ifa_addr->sa_family == AF_INET6)
			{
				struct sockaddr_in6* s6 = (struct sockaddr_in6*)(ifa->ifa_addr);
				CNetAddr addr(s6->sin6_addr);
				if (AddLocal(addr, LOCAL_IF))
					LogPrintf("IPv6 %s: %s\n", ifa->ifa_name, addr.ToString());
			}
#endif
		}
		freeifaddrs(myaddrs);
	}
#endif

	// Don't use external IPv4 discovery, when -onlynet="IPv6"
//	if (!IsLimited(NET_IPV4))
//		threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "ext-ip", &ThreadGetMyExternalIP));
}

// learn a new local address
bool
CManageNetwork::addLocal(const CService& addr, int nScore)
{
	if (!addr.IsRoutable())
		return false;

	if (!fDiscover && nScore < LOCAL_MANUAL)
		return false;

	if (IsLimited(addr))
		return false;

	LogPrintf("AddLocal(%s,%i)\n", addr.ToString(), nScore);

	{
		LOCK(cs_mapLocalHost);
		bool fAlready = mapLocalHost.count(addr) > 0;
		LocalServiceInfo &info = mapLocalHost[addr];
		if (!fAlready || nScore >= info.nScore) {
			info.nScore = nScore + (fAlready ? 1 : 0);
			info.nPort = addr.GetPort();
		}
		SetReachable(addr.GetNetwork());
	}

	advertizeLocal();

	return true;
}

bool 
CManageNetwork::addLocal(const CNetAddr &addr, int nScore)
{
	return AddLocal(CService(addr, GetListenPort< common::CRatcoinParams >()), nScore);
}

void
CManageNetwork::advertizeLocal()
{
	LOCK(cs_vNodes);
	BOOST_FOREACH(CNode* pnode, m_nodes)
	{
		if (pnode->fSuccessfullyConnected)
		{
			CAddress addrLocal = GetLocalAddress(&pnode->addr);
			if (addrLocal.IsRoutable() && (CService)addrLocal != (CService)pnode->addrLocal)
			{
				pnode->PushAddress(addrLocal);
				pnode->addrLocal = addrLocal;
			}
		}
	}
}

void 
CManageNetwork::threadSocketHandler()
{
	unsigned int nPrevNodeCount = 0;
	while (true)
	{
		//
		// Disconnect nodes
		//
		{
			LOCK(cs_vNodes);
			// Disconnect unused nodes
			vector<CNode*> vNodesCopy = m_nodes;
			BOOST_FOREACH(CNode* pnode, vNodesCopy)
			{
				if (pnode->fDisconnect ||
					(pnode->GetRefCount() <= 0 && pnode->vRecvMsg.empty() && pnode->nSendSize == 0 && pnode->ssSend.empty())
				/*	|| !m_nodesManager->isNodeHonest( pnode )*/)
				{
					// remove from m_nodes
					m_nodes.erase(remove(m_nodes.begin(), m_nodes.end(), pnode), m_nodes.end());

					// release outbound grant (if any)
					pnode->grantOutbound.Release();

					// close socket and cleanup
					pnode->CloseSocketDisconnect();
					pnode->Cleanup();

					// hold in disconnected pool until all refs are released
					if (pnode->fNetworkNode || pnode->fInbound)
						pnode->Release();
					m_nodesDisconnected.push_back(pnode);
				}
			}
		}
		{
			// Delete disconnected nodes
			list<CNode*> vNodesDisconnectedCopy = m_nodesDisconnected;
			BOOST_FOREACH(CNode* pnode, vNodesDisconnectedCopy)
			{
				// wait until threads are done using it
				if (pnode->GetRefCount() <= 0)
				{
					bool fDelete = false;
					{
						TRY_LOCK(pnode->cs_vSend, lockSend);
						if (lockSend)
						{
							TRY_LOCK(pnode->cs_vRecvMsg, lockRecv);
							if (lockRecv)
							{
								TRY_LOCK(pnode->cs_inventory, lockInv);
								if (lockInv)
									fDelete = true;
							}
						}
					}
					if (fDelete)
					{
						m_nodesDisconnected.remove(pnode);
						delete pnode;
					}
				}
			}
		}
		if(m_nodes.size() != nPrevNodeCount) {
			nPrevNodeCount = m_nodes.size();
			//uiInterface.NotifyNumConnectionsChanged(nPrevNodeCount);  not  needed now
		}

		//
		// Find which sockets have data to receive
		//
		struct timeval timeout;
		timeout.tv_sec  = 0;
		timeout.tv_usec = 50000; // frequency to poll pnode->vSend

		fd_set fdsetRecv;
		fd_set fdsetSend;
		fd_set fdsetError;
		FD_ZERO(&fdsetRecv);
		FD_ZERO(&fdsetSend);
		FD_ZERO(&fdsetError);
		SOCKET hSocketMax = 0;
		bool have_fds = false;

		BOOST_FOREACH(SOCKET hListenSocket, m_listenSocket) {
			FD_SET(hListenSocket, &fdsetRecv);
			hSocketMax = max(hSocketMax, hListenSocket);
			have_fds = true;
		}
		{
			LOCK(cs_vNodes);
			BOOST_FOREACH(CNode* pnode, m_nodes)
			{
				if (pnode->hSocket == INVALID_SOCKET)
					continue;
				FD_SET(pnode->hSocket, &fdsetError);
				hSocketMax = max(hSocketMax, pnode->hSocket);
				have_fds = true;

				// Implement the following logic:
				// * If there is data to send, select() for sending data. As this only
				//   happens when optimistic write failed, we choose to first drain the
				//   write buffer in this case before receiving more. This avoids
				//   needlessly queueing received data, if the remote peer is not themselves
				//   receiving data. This means properly utilizing TCP flow control signalling.
				// * Otherwise, if there is no (complete) message in the receive buffer,
				//   or there is space left in the buffer, select() for receiving data.
				// * (if neither of the above applies, there is certainly one message
				//   in the receiver buffer ready to be processed).
				// Together, that means that at least one of the following is always possible,
				// so we don't deadlock:
				// * We send some data.
				// * We wait for data to be received (and disconnect after timeout).
				// * We process a message in the buffer (message handler thread).
				{
					TRY_LOCK(pnode->cs_vSend, lockSend);
					if (lockSend && !pnode->vSendMsg.empty()) {
						FD_SET(pnode->hSocket, &fdsetSend);
						continue;
					}
				}
				{
					TRY_LOCK(pnode->cs_vRecvMsg, lockRecv);
					if (lockRecv && (
						pnode->vRecvMsg.empty() || !pnode->vRecvMsg.front().complete() ||
						pnode->GetTotalRecvSize() <= ReceiveFloodSize()))
						FD_SET(pnode->hSocket, &fdsetRecv);
				}
			}
		}

		int nSelect = select(have_fds ? hSocketMax + 1 : 0,
			&fdsetRecv, &fdsetSend, &fdsetError, &timeout);
		boost::this_thread::interruption_point();

		if (nSelect == SOCKET_ERROR)
		{
			if (have_fds)
			{
				int nErr = WSAGetLastError();
				LogPrintf("socket select error %d\n", nErr);
				for (unsigned int i = 0; i <= hSocketMax; i++)
					FD_SET(i, &fdsetRecv);
			}
			FD_ZERO(&fdsetSend);
			FD_ZERO(&fdsetError);
			MilliSleep(timeout.tv_usec/1000);
		}


		//
		// Accept new connections
		//
		BOOST_FOREACH(SOCKET hListenSocket, m_listenSocket)
			if (hListenSocket != INVALID_SOCKET && FD_ISSET(hListenSocket, &fdsetRecv))
			{
#ifdef USE_IPV6
				struct sockaddr_storage sockaddr;
#else
				struct sockaddr sockaddr;
#endif
				socklen_t len = sizeof(sockaddr);
				SOCKET hSocket = accept(hListenSocket, (struct sockaddr*)&sockaddr, &len);
				CAddress addr;
				int nInbound = 0;

				if (hSocket != INVALID_SOCKET)
					if (!addr.SetSockAddr((const struct sockaddr*)&sockaddr))
						LogPrintf("Warning: Unknown socket family\n");

				{
					LOCK(cs_vNodes);
					BOOST_FOREACH(CNode* pnode, m_nodes)
						if (pnode->fInbound)
							nInbound++;
				}

				if (hSocket == INVALID_SOCKET)
				{
					int nErr = WSAGetLastError();
					if (nErr != WSAEWOULDBLOCK)
						LogPrintf("socket error accept failed: %d\n", nErr);
				}
				else if (nInbound >= nMaxConnections - MAX_OUTBOUND_CONNECTIONS)
				{
					{
						LOCK(cs_setservAddNodeAddresses);
						if (!setservAddNodeAddresses.count(addr))
							closesocket(hSocket);
					}
				}
			/*	else if (m_nodesManager->isBanned( addr ))
				{
					LogPrintf("connection from %s dropped (banned)\n", addr.ToString());
					closesocket(hSocket);
				}*/
				else
				{
					LogPrint("net", "accepted connection %s\n", addr.ToString());
					CNode* pnode = new CNode(hSocket, addr, "", true);
					pnode->AddRef();
					{
						LOCK(cs_vNodes);
						m_nodes.push_back(pnode);
					}
				}
			}


			//
			// Service each socket
			//
			vector<CNode*> vNodesCopy;
			{
				LOCK(cs_vNodes);
				vNodesCopy = m_nodes;
				BOOST_FOREACH(CNode* pnode, vNodesCopy)
					pnode->AddRef();
			}
			BOOST_FOREACH(CNode* pnode, vNodesCopy)
			{
				boost::this_thread::interruption_point();

				//
				// Receive
				//
				if (pnode->hSocket == INVALID_SOCKET)
					continue;
				if (FD_ISSET(pnode->hSocket, &fdsetRecv) || FD_ISSET(pnode->hSocket, &fdsetError))
				{
					TRY_LOCK(pnode->cs_vRecvMsg, lockRecv);
					if (lockRecv)
					{
						{
							// typical socket buffer is 8K-64K
							char pchBuf[0x10000];
							int nBytes = recv(pnode->hSocket, pchBuf, sizeof(pchBuf), MSG_DONTWAIT);
							if (nBytes > 0)
							{
								if (!pnode->ReceiveMsgBytes(pchBuf, nBytes))
									pnode->CloseSocketDisconnect();
								pnode->nLastRecv = GetTime();
								pnode->nRecvBytes += nBytes;
								pnode->RecordBytesRecv(nBytes);
							}
							else if (nBytes == 0)
							{
								// socket closed gracefully
								if (!pnode->fDisconnect)
									LogPrint("net", "socket closed\n");
								pnode->CloseSocketDisconnect();
							}
							else if (nBytes < 0)
							{
								// error
								int nErr = WSAGetLastError();
								if (nErr != WSAEWOULDBLOCK && nErr != WSAEMSGSIZE && nErr != WSAEINTR && nErr != WSAEINPROGRESS)
								{
									if (!pnode->fDisconnect)
										LogPrintf("socket recv error %d\n", nErr);
									pnode->CloseSocketDisconnect();
								}
							}
						}
					}
				}

				//
				// Send
				//
				if (pnode->hSocket == INVALID_SOCKET)
					continue;
				if (FD_ISSET(pnode->hSocket, &fdsetSend))
				{
					TRY_LOCK(pnode->cs_vSend, lockSend);
					if (lockSend)
						SocketSendData(pnode);
				}

				//
				// Inactivity checking
				//
				if (pnode->vSendMsg.empty())
					pnode->nLastSendEmpty = GetTime();
				if (GetTime() - pnode->nTimeConnected > 60)
				{
					if (pnode->nLastRecv == 0 || pnode->nLastSend == 0)
					{
						LogPrint("net", "socket no message in first 60 seconds, %d %d\n", pnode->nLastRecv != 0, pnode->nLastSend != 0);
						pnode->fDisconnect = true;
					}
					else if (GetTime() - pnode->nLastSend > 90*60 && GetTime() - pnode->nLastSendEmpty > 90*60)
					{
						LogPrintf("socket not sending\n");
						pnode->fDisconnect = true;
					}
					else if (GetTime() - pnode->nLastRecv > 90*60)
					{
						LogPrintf("socket inactivity timeout\n");
						pnode->fDisconnect = true;
					}
				}
			}
			{
				LOCK(cs_vNodes);
				BOOST_FOREACH(CNode* pnode, vNodesCopy)
					pnode->Release();
			}

			MilliSleep(10);
	}
}


void 
CManageNetwork::threadOpenAddedConnections()
{
	{
		LOCK(cs_vAddedNodes);
		vAddedNodes = mapMultiArgs["-addnode"];
	}

	if (HaveNameProxy()) {
		while(true) {
			list<string> lAddresses(0);
			{
				LOCK(cs_vAddedNodes);
				BOOST_FOREACH(string& strAddNode, vAddedNodes)
					lAddresses.push_back(strAddNode);
			}
			BOOST_FOREACH(string& strAddNode, lAddresses) {
				CAddress addr;
				CSemaphoreGrant grant(*m_semOutbound);
				//OpenNetworkConnection(addr, &grant, strAddNode.c_str());
				MilliSleep(500);
			}
			MilliSleep(120000); // Retry every 2 minutes
		}
	}

	for (unsigned int i = 0; true; i++)
	{
		list<string> lAddresses(0);
		{
			LOCK(cs_vAddedNodes);
			BOOST_FOREACH(string& strAddNode, vAddedNodes)
				lAddresses.push_back(strAddNode);
		}

		list<vector<CService> > lservAddressesToAdd(0);
		BOOST_FOREACH(string& strAddNode, lAddresses)
		{
			vector<CService> vservNode(0);
			if(Lookup(strAddNode.c_str(), vservNode, GetNetworkParams< common::CRatcoinParams >().GetDefaultPort(), fNameLookup, 0))
			{
				lservAddressesToAdd.push_back(vservNode);
				{
					LOCK(cs_setservAddNodeAddresses);
					BOOST_FOREACH(CService& serv, vservNode)
						setservAddNodeAddresses.insert(serv);
				}
			}
		}
		// Attempt to connect to each IP for each addnode entry until at least one is successful per addnode entry
		// (keeping in mind that addnode entries can have many IPs if fNameLookup)
		{
			LOCK(cs_vNodes);
			BOOST_FOREACH(CNode* pnode, m_nodes)
				for (list<vector<CService> >::iterator it = lservAddressesToAdd.begin(); it != lservAddressesToAdd.end(); it++)
					BOOST_FOREACH(CService& addrNode, *(it))
					if (pnode->addr == addrNode)
					{
						it = lservAddressesToAdd.erase(it);
						it--;
						break;
					}
		}
		BOOST_FOREACH(vector<CService>& vserv, lservAddressesToAdd)
		{
			CSemaphoreGrant grant(*m_semOutbound);
//			OpenNetworkConnection(CAddress(vserv[i % vserv.size()]), &grant);
			MilliSleep(500);
		}
		MilliSleep(120000); // Retry every 2 minutes
	}
}


void
CManageNetwork::threadOpenConnections()
{
	// Connect to specific addresses
	if (mapArgs.count("-connect") && mapMultiArgs["-connect"].size() > 0)
	{
		for (int64_t nLoop = 0;; nLoop++)
		{
//			ProcessOneShot();
			BOOST_FOREACH(string strAddr, mapMultiArgs["-connect"])
			{
				CAddress addr;
				openNetworkConnection(addr, NULL, strAddr.c_str());
				for (int i = 0; i < 10 && i < nLoop; i++)
				{
					MilliSleep(500);
				}
			}
			MilliSleep(500);
		}
	}

	// Initiate network connections
	int64_t nStart = GetTime();
	while (true)
	{
		//ProcessOneShot();

		MilliSleep(500);

		CSemaphoreGrant grant(*m_semOutbound);
		boost::this_thread::interruption_point();

		// Add seed nodes if DNS seeds are all down (an infrastructure attack?).
		if (addrman.size() == 0 && (GetTime() - nStart > 60)) {
			static bool done = false;
			if (!done) {
				LogPrintf("Adding fixed seed nodes as DNS doesn't seem to be available.\n");
				addrman.Add(Params().FixedSeeds(), CNetAddr("127.0.0.1"));
				done = true;
			}
		}

		//
		// Choose an address to connect to based on most recently seen
		//
		CAddress addrConnect;

		// Only connect out to one peer per network group (/16 for IPv4).
		// Do this here so we don't have to critsect m_nodes inside mapAddresses critsect.
		int nOutbound = 0;
		set<vector<unsigned char> > setConnected;
		{
			LOCK(cs_vNodes);
			BOOST_FOREACH(CNode* pnode, m_nodes) {
				if (!pnode->fInbound) {
					setConnected.insert(pnode->addr.GetGroup());
					nOutbound++;
				}
			}
		}

		int64_t nANow = GetAdjustedTime();

		int nTries = 0;
		while (true)
		{
			// use an nUnkBias between 10 (no outgoing connections) and 90 (8 outgoing connections)
			CAddress addr = addrman.Select(10 + min(nOutbound,8)*10);

			// if we selected an invalid address, restart
			if (!addr.IsValid() || setConnected.count(addr.GetGroup()) || IsLocal(addr))
				break;

			// If we didn't find an appropriate destination after trying 100 addresses fetched from addrman,
			// stop this loop, and let the outer loop run again (which sleeps, adds seed nodes, recalculates
			// already-connected network ranges, ...) before trying new addrman addresses.
			nTries++;
			if (nTries > 100)
				break;

			if (IsLimited(addr))
				continue;

			// only consider very recently tried nodes after 30 failed attempts
			if (nANow - addr.nLastTry < 600 && nTries < 30)
				continue;

			// do not allow non-default ports, unless after 50 invalid addresses selected already
			if (addr.GetPort() != GetNetworkParams< common::CRatcoinParams >().GetDefaultPort() && nTries < 50)
				continue;

			addrConnect = addr;
			break;
		}

	//	if (addrConnect.IsValid())
	//		openNetworkConnection(addrConnect, &grant);
	}
}

CNode*
CManageNetwork::findNode(const CNetAddr& ip)
{
	LOCK(cs_vNodes);
	BOOST_FOREACH(CNode* pnode, m_nodes)
		if ((CNetAddr)pnode->addr == ip)
			return (pnode);
	return NULL;
}


bool
CManageNetwork::bind(const CService &addr, unsigned int flags)
{
	if (!(flags & BF_EXPLICIT) && IsLimited(addr))
		return false;
	std::string strError;
	if (!bindListenPort(addr, strError)) {
		if (flags & BF_REPORT_ERROR)
			return false;
		return false;
	}
	return true;
}

bool
CManageNetwork::bindListenPort(const CService &addrBind, string& strError)
{
	strError = "";
	int nOne = 1;

	// Create socket for listening for incoming connections
#ifdef USE_IPV6
	struct sockaddr_storage sockaddr;
#else
	struct sockaddr sockaddr;
#endif
	socklen_t len = sizeof(sockaddr);
	if (!addrBind.GetSockAddr((struct sockaddr*)&sockaddr, &len))
	{
		strError = strprintf("Error: bind address family for %s not supported", addrBind.ToString());
		LogPrintf("%s\n", strError);
		return false;
	}

	SOCKET hListenSocket = socket(((struct sockaddr*)&sockaddr)->sa_family, SOCK_STREAM, IPPROTO_TCP);
	if (hListenSocket == INVALID_SOCKET)
	{
		strError = strprintf("Error: Couldn't open socket for incoming connections (socket returned error %d)", WSAGetLastError());
		LogPrintf("%s\n", strError);
		return false;
	}

#ifdef SO_NOSIGPIPE
	// Different way of disabling SIGPIPE on BSD
	setsockopt(hListenSocket, SOL_SOCKET, SO_NOSIGPIPE, (void*)&nOne, sizeof(int));
#endif

#ifndef WIN32
	// Allow binding if the port is still in TIME_WAIT state after
	// the program was closed and restarted.  Not an issue on windows.
	setsockopt(hListenSocket, SOL_SOCKET, SO_REUSEADDR, (void*)&nOne, sizeof(int));
#endif


#ifdef WIN32
	// Set to non-blocking, incoming connections will also inherit this
	if (ioctlsocket(hListenSocket, FIONBIO, (u_long*)&nOne) == SOCKET_ERROR)
#else
	if (fcntl(hListenSocket, F_SETFL, O_NONBLOCK) == SOCKET_ERROR)
#endif
	{
		strError = strprintf("Error: Couldn't set properties on socket for incoming connections (error %d)", WSAGetLastError());
		LogPrintf("%s\n", strError);
		return false;
	}

#ifdef USE_IPV6
	// some systems don't have IPV6_V6ONLY but are always v6only; others do have the option
	// and enable it by default or not. Try to enable it, if possible.
	if (addrBind.IsIPv6()) {
#ifdef IPV6_V6ONLY
#ifdef WIN32
		setsockopt(hListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (const char*)&nOne, sizeof(int));
#else
		setsockopt(hListenSocket, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&nOne, sizeof(int));
#endif
#endif
#ifdef WIN32
		int nProtLevel = 10 /* PROTECTION_LEVEL_UNRESTRICTED */;
		int nParameterId = 23 /* IPV6_PROTECTION_LEVEl */;
		// this call is allowed to fail
		setsockopt(hListenSocket, IPPROTO_IPV6, nParameterId, (const char*)&nProtLevel, sizeof(int));
#endif
	}
#endif

	if (::bind(hListenSocket, (struct sockaddr*)&sockaddr, len) == SOCKET_ERROR)
	{
		int nErr = WSAGetLastError();
		if (nErr == WSAEADDRINUSE)
			strError = strprintf("Unable to bind to %s on this computer. Bitcoin is probably already running. %s", addrBind.ToString());
		else
			strError = strprintf("Unable to bind to %s on this computer (bind returned error %d, %s)", addrBind.ToString(), nErr, strerror(nErr));
		LogPrintf("%s\n", strError);
		return false;
	}
	LogPrintf("Bound to %s\n", addrBind.ToString());

	// Listen for incoming connections
	if (listen(hListenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		strError = strprintf("Error: Listening for incoming connections failed (listen returned error %d)", WSAGetLastError());
		LogPrintf("%s\n", strError);
		return false;
	}

	m_listenSocket.push_back(hListenSocket);

	if (addrBind.IsRoutable() && fDiscover)
		AddLocal(addrBind, LOCAL_BIND);

	return true;
}

CNode*
CManageNetwork::findNode(std::string addrName)
{
	LOCK(cs_vNodes);
	BOOST_FOREACH(CNode* pnode, m_nodes)
		if (pnode->addrName == addrName)
			return (pnode);
	return NULL;
}

CNode*
CManageNetwork::findNode(const CService& addr)
{
	LOCK(cs_vNodes);
	BOOST_FOREACH(CNode* pnode, m_nodes)
		if ((CService)pnode->addr == addr)
			return (pnode);
	return NULL;
}

bool
CManageNetwork::openNetworkConnection(const CAddress& addrConnect, CSemaphoreGrant *grantOutbound, const char *strDest, bool fOneShot)
{
	//
	// Initiate outbound network connection
	//
	boost::this_thread::interruption_point();
/*	if (!strDest)
		if (IsLocal(addrConnect) ||
			FindNode((CNetAddr)addrConnect) || CNode::IsBanned(addrConnect) ||
			FindNode(addrConnect.ToStringIPPort().c_str()))
			return false;*/
//	if (strDest && FindNode(strDest))
//		return false;

	CNode* pnode = connectNode(addrConnect, strDest);
	boost::this_thread::interruption_point();

	if (!pnode)
		return false;
	if (grantOutbound)
		grantOutbound->MoveTo(pnode->grantOutbound);
	pnode->fNetworkNode = true;
	if (fOneShot)
		pnode->fOneShot = true;

	return true;
}

CNode*
CManageNetwork::connectNode(CAddress addrConnect, const char *pszDest)
{
	if (pszDest == NULL) {
		if (IsLocal(addrConnect))
			return NULL;

		// Look for an existing connection
		CNode* pnode = FindNode((CService)addrConnect);
		if (pnode)
		{
			pnode->AddRef();
			return pnode;
		}
	}


	/// debug print
	LogPrint("net", "trying connection %s lastseen=%.1fhrs\n",
		pszDest ? pszDest : addrConnect.ToString(),
		pszDest ? 0 : (double)(GetAdjustedTime() - addrConnect.nTime)/3600.0);

	// Connect
	SOCKET hSocket;
	if (pszDest ? ConnectSocketByName(addrConnect, hSocket, pszDest, GetNetworkParams< common::CRatcoinParams >().GetDefaultPort()) : ConnectSocket(addrConnect, hSocket))
	{
		addrman.Attempt(addrConnect);

		LogPrint("net", "connected %s\n", pszDest ? pszDest : addrConnect.ToString());

		// Set to non-blocking
#ifdef WIN32
		u_long nOne = 1;
		if (ioctlsocket(hSocket, FIONBIO, &nOne) == SOCKET_ERROR)
			LogPrintf("ConnectSocket() : ioctlsocket non-blocking setting failed, error %d\n", WSAGetLastError());
#else
		if (fcntl(hSocket, F_SETFL, O_NONBLOCK) == SOCKET_ERROR)
			LogPrintf("ConnectSocket() : fcntl non-blocking setting failed, error %d\n", errno);
#endif

		// Add node
		CNode* pnode = new CNode(hSocket, addrConnect, pszDest ? pszDest : "", false);
		pnode->AddRef();

		{
			LOCK(cs_vNodes);
			m_nodes.push_back(pnode);
		}

		pnode->nTimeConnected = GetTime();
		return pnode;
	}
	else
	{
		return NULL;
	}
}

void
CManageNetwork::StartSync(const vector<CNode*> &vNodes)
{
	CNode *pnodeNewSync = NULL;
	double dBestScore = 0;

	// Iterate over all nodes
	BOOST_FOREACH(CNode* pnode, m_nodes) {
		// check preconditions for allowing a sync
		if (!pnode->fClient && !pnode->fOneShot &&
			!pnode->fDisconnect && pnode->fSuccessfullyConnected &&
			(pnode->nVersion < NOBLKS_VERSION_START || pnode->nVersion >= NOBLKS_VERSION_END)) {
			// if ok, compare node's score with the best so far
		//	double dScore = NodeSyncScore(pnode);
/*			if (pnodeNewSync == NULL || dScore > dBestScore) {
				pnodeNewSync = pnode;
				dBestScore = dScore;
			}*/
		}
	}
	// if a new sync candidate was found, start sync!
	if (pnodeNewSync) {
		pnodeNewSync->fStartSync = true;
		pnodeSync = pnodeNewSync;
	}
}

void 
CManageNetwork::threadMessageHandler()
{
	SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
	while (true)
	{
		bool fHaveSyncNode = false;

		vector<CNode*> vNodesCopy;
		{
			LOCK(cs_vNodes);
			vNodesCopy = m_nodes;
			BOOST_FOREACH(CNode* pnode, vNodesCopy) {
				pnode->AddRef();
				if (pnode == pnodeSync)
					fHaveSyncNode = true;
			}
		}

		if (!fHaveSyncNode)
			StartSync(vNodesCopy);

		// Poll the connected nodes for messages
		CNode* pnodeTrickle = NULL;
		if (!vNodesCopy.empty())
			pnodeTrickle = vNodesCopy[GetRand(vNodesCopy.size())];

		bool fSleep = true;

		BOOST_FOREACH(CNode* pnode, vNodesCopy)
		{
			if (pnode->fDisconnect)
				continue;

			// Receive messages
			{
				TRY_LOCK(pnode->cs_vRecvMsg, lockRecv);
				if (lockRecv)
				{
					if (!m_signals.ProcessMessages(pnode))
						pnode->CloseSocketDisconnect();

					if (pnode->nSendSize < SendBufferSize())
					{
						if (!pnode->vRecvGetData.empty() || (!pnode->vRecvMsg.empty() && pnode->vRecvMsg[0].complete()))
						{
							fSleep = false;
						}
					}
				}
			}
			boost::this_thread::interruption_point();


/*
 * // Send messages
nodes manager
- trackers
- monitors
*/

			{
				TRY_LOCK(pnode->cs_vSend, lockSend);
				if (lockSend)
					m_signals.SendMessages(pnode, pnode == pnodeTrickle);
			}
			boost::this_thread::interruption_point();
		}

		{
			LOCK(cs_vNodes);
			BOOST_FOREACH(CNode* pnode, vNodesCopy)
				pnode->Release();
		}

		if (fSleep)
			MilliSleep(100);
	}
}




void
CManageNetwork::processGetData(CNode* pfrom)
{

	std::deque<CInv>::iterator it = pfrom->vRecvGetData.begin();
	pfrom->vRecvGetData.erase(pfrom->vRecvGetData.begin(), it);


	pfrom->PushMessage("notfound",  vector<CInv>());

}

void
CManageNetwork::registerNodeSignals(CNodeSignals& nodeSignals)
{
	boost::bind( &CManageNetwork::processMessages, this );

	m_signals.ProcessMessages.connect(boost::bind( &CManageNetwork::processMessages, this, _1 ));
	m_signals.SendMessages.connect(boost::bind( &CManageNetwork::sendMessages, this, _1, _2 ) );
//	m_signals.InitializeNode.connect(boost::bind( &CManageNetwork::initializeNode, this ));
//	m_signals.FinalizeNode.connect(boost::bind( &CManageNetwork::finalizeNode, this ));
}

void
CManageNetwork::unregisterNodeSignals(CNodeSignals& nodeSignals)
{
	m_signals.ProcessMessages.disconnect(boost::bind( &CManageNetwork::processMessages, this, _1 ));
	m_signals.SendMessages.disconnect(boost::bind( &CManageNetwork::sendMessages, this, _1, _2 ) );
//	m_signals.InitializeNode.disconnect(boost::bind( &CManageNetwork::initializeNode, this ));
//	m_signals.FinalizeNode.disconnect(boost::bind( &CManageNetwork::finalizeNode, this ));
}

bool
CManageNetwork::processMessage(CNode* pfrom, CDataStream& vRecv)
{
	std::vector< CMessage > messages;
	vRecv >> messages;
	CNodesManager::getInstance()->processMessagesFormNode( pfrom, messages );

	/*

	RandAddSeedPerfmon();
	LogPrint("net", "received: %s (%"PRIszu" bytes)\n", strCommand, vRecv.size());
	if (mapArgs.count("-dropmessagestest") && GetRand(atoi(mapArgs["-dropmessagestest"])) == 0)
	{
		LogPrintf("dropmessagestest DROPPING RECV MESSAGE\n");
		return true;
	}





	if (strCommand == "version")
	{
		// Each connection can only send one version message
		if (pfrom->nVersion != 0)
		{
		//	pfrom->PushMessage("reject", strCommand, REJECT_DUPLICATE, string("Duplicate version message"));
		//	Misbehaving(pfrom->GetId(), 1);
			return false;
		}



		pfrom->fClient = !(pfrom->nServices & NODE_NETWORK);


		// Relay alerts
		{
			LOCK(cs_mapAlerts);
			BOOST_FOREACH(PAIRTYPE(const uint256, CAlert)& item, mapAlerts)
				item.second.RelayTo(pfrom);
		}

		pfrom->fSuccessfullyConnected = true;

		LogPrintf("receive version message: %s: version %d, blocks=%d, us=%s, them=%s, peer=%s\n", pfrom->cleanSubVer, pfrom->nVersion, pfrom->nStartingHeight, addrMe.ToString(), addrFrom.ToString(), pfrom->addr.ToString());

		AddTimeData(pfrom->addr, nTime);

		LOCK(cs_main);
		cPeerBlockCounts.input(pfrom->nStartingHeight);
	}


	else if (pfrom->nVersion == 0)
	{
		// Must have a version message before anything else
		Misbehaving(pfrom->GetId(), 1);
		return false;
	}


	else if (strCommand == "verack")
	{
		pfrom->SetRecvVersion(min(pfrom->nVersion, PROTOCOL_VERSION));
	}


	else if (strCommand == "addr")
	{
		vector<CAddress> vAddr;
		vRecv >> vAddr;

		// Don't want addr from older versions unless seeding
		if (pfrom->nVersion < CADDR_TIME_VERSION && addrman.size() > 1000)
			return true;
		if (vAddr.size() > 1000)
		{
			Misbehaving(pfrom->GetId(), 20);
			return error("message addr size() = %"PRIszu"", vAddr.size());
		}

		// Store the new addresses
		vector<CAddress> vAddrOk;
		int64_t nNow = GetAdjustedTime();
		int64_t nSince = nNow - 10 * 60;
		BOOST_FOREACH(CAddress& addr, vAddr)
		{
			boost::this_thread::interruption_point();

			if (addr.nTime <= 100000000 || addr.nTime > nNow + 10 * 60)
				addr.nTime = nNow - 5 * 24 * 60 * 60;
			pfrom->AddAddressKnown(addr);
			bool fReachable = IsReachable(addr);
			if (addr.nTime > nSince && !pfrom->fGetAddr && vAddr.size() <= 10 && addr.IsRoutable())
			{
				// Relay to a limited number of other nodes
				{
					LOCK(cs_vNodes);
					// Use deterministic randomness to send to the same nodes for 24 hours
					// at a time so the setAddrKnowns of the chosen nodes prevent repeats
					static uint256 hashSalt;
					if (hashSalt == 0)
						hashSalt = GetRandHash();
					uint64_t hashAddr = addr.GetHash();
					uint256 hashRand = hashSalt ^ (hashAddr<<32) ^ ((GetTime()+hashAddr)/(24*60*60));
					hashRand = Hash(BEGIN(hashRand), END(hashRand));
					multimap<uint256, CNode*> mapMix;
					BOOST_FOREACH(CNode* pnode, vNodes)
					{
						if (pnode->nVersion < CADDR_TIME_VERSION)
							continue;
						unsigned int nPointer;
						memcpy(&nPointer, &pnode, sizeof(nPointer));
						uint256 hashKey = hashRand ^ nPointer;
						hashKey = Hash(BEGIN(hashKey), END(hashKey));
						mapMix.insert(make_pair(hashKey, pnode));
					}
					int nRelayNodes = fReachable ? 2 : 1; // limited relaying of addresses outside our network(s)
					for (multimap<uint256, CNode*>::iterator mi = mapMix.begin(); mi != mapMix.end() && nRelayNodes-- > 0; ++mi)
						((*mi).second)->PushAddress(addr);
				}
			}
			// Do not store addresses outside our network
			if (fReachable)
				vAddrOk.push_back(addr);
		}
		addrman.Add(vAddrOk, pfrom->addr, 2 * 60 * 60);
		if (vAddr.size() < 1000)
			pfrom->fGetAddr = false;
		if (pfrom->fOneShot)
			pfrom->fDisconnect = true;
	}


	else if (strCommand == "getaddr")
	{
		pfrom->vAddrToSend.clear();
		vector<CAddress> vAddr = addrman.GetAddr();
		BOOST_FOREACH(const CAddress &addr, vAddr)
			pfrom->PushAddress(addr);
	}


	else if (strCommand == "ping")
	{
		if (pfrom->nVersion > BIP0031_VERSION)
		{
			uint64_t nonce = 0;
			vRecv >> nonce;
			// Echo the message back with the nonce. This allows for two useful features:
			//
			// 1) A remote node can quickly check if the connection is operational
			// 2) Remote nodes can measure the latency of the network thread. If this node
			//    is overloaded it won't respond to pings quickly and the remote node can
			//    avoid sending us more work, like chain download requests.
			//
			// The nonce stops the remote getting confused between different pings: without
			// it, if the remote node sends a ping once per second and this node takes 5
			// seconds to respond to each, the 5th ping the remote sends would appear to
			// return very quickly.
			pfrom->PushMessage("pong", nonce);
		}
	}


	else if (strCommand == "pong")
	{
		int64_t pingUsecEnd = GetTimeMicros();
		uint64_t nonce = 0;
		size_t nAvail = vRecv.in_avail();
		bool bPingFinished = false;
		std::string sProblem;

		if (nAvail >= sizeof(nonce)) {
			vRecv >> nonce;

			// Only process pong message if there is an outstanding ping (old ping without nonce should never pong)
			if (pfrom->nPingNonceSent != 0) {
				if (nonce == pfrom->nPingNonceSent) {
					// Matching pong received, this ping is no longer outstanding
					bPingFinished = true;
					int64_t pingUsecTime = pingUsecEnd - pfrom->nPingUsecStart;
					if (pingUsecTime > 0) {
						// Successful ping time measurement, replace previous
						pfrom->nPingUsecTime = pingUsecTime;
					} else {
						// This should never happen
						sProblem = "Timing mishap";
					}
				} else {
					// Nonce mismatches are normal when pings are overlapping
					sProblem = "Nonce mismatch";
					if (nonce == 0) {
						// This is most likely a bug in another implementation somewhere, cancel this ping
						bPingFinished = true;
						sProblem = "Nonce zero";
					}
				}
			} else {
				sProblem = "Unsolicited pong without ping";
			}
		} else {
			// This is most likely a bug in another implementation somewhere, cancel this ping
			bPingFinished = true;
			sProblem = "Short payload";
		}

		if (!(sProblem.empty())) {
			LogPrint("net", "pong %s %s: %s, %"PRIx64" expected, %"PRIx64" received, %"PRIszu" bytes\n",
				pfrom->addr.ToString(),
				pfrom->cleanSubVer,
				sProblem,
				pfrom->nPingNonceSent,
				nonce,
				nAvail);
		}
		if (bPingFinished) {
			pfrom->nPingNonceSent = 0;
		}
	}


	else if (strCommand == "alert")
	{
		CAlert alert;
		vRecv >> alert;

		uint256 alertHash = alert.GetHash();
		if (pfrom->setKnown.count(alertHash) == 0)
		{
			if (alert.ProcessAlert())
			{
				// Relay
				pfrom->setKnown.insert(alertHash);
				{
					LOCK(cs_vNodes);
					BOOST_FOREACH(CNode* pnode, vNodes)
						alert.RelayTo(pnode);
				}
			}
			else {
				// Small DoS penalty so peers that send us lots of
				// duplicate/expired/invalid-signature/whatever alerts
				// eventually get banned.
				// This isn't a Misbehaving(100) (immediate ban) because the
				// peer might be an older or different implementation with
				// a different signature key, etc.
				Misbehaving(pfrom->GetId(), 10);
			}
		}
	}



	// Update the last seen time for this node's address
	if (pfrom->fNetworkNode)
		if (strCommand == "version" || strCommand == "addr" || strCommand == "inv" || strCommand == "getdata" || strCommand == "ping")
			AddressCurrentlyConnected(pfrom->addr);

*/
	return true;
}

// requires LOCK(cs_vRecvMsg)
bool
CManageNetwork::processMessages(CNode* pfrom)
{
	//if (fDebug)
	//    LogPrintf("ProcessMessages(%"PRIszu" messages)\n", pfrom->vRecvMsg.size());

	//
	// Message format
	//  (4) message start
	//  (12) command
	//  (4) size
	//  (4) checksum
	//  (x) data
	//
	bool fOk = true;

	if (!pfrom->vRecvGetData.empty())
		processGetData(pfrom);

	// this maintains the order of responses
	if (!pfrom->vRecvGetData.empty()) return fOk;

	std::deque<CNetMessage>::iterator it = pfrom->vRecvMsg.begin();
	while (!pfrom->fDisconnect && it != pfrom->vRecvMsg.end()) {
		// Don't bother if send buffer is too full to respond anyway
		if (pfrom->nSendSize >= SendBufferSize())
			break;

		// get next message
		CNetMessage& msg = *it;

		//if (fDebug)
		//    LogPrintf("ProcessMessages(message %u msgsz, %"PRIszu" bytes, complete:%s)\n",
		//            msg.hdr.nMessageSize, msg.vRecv.size(),
		//            msg.complete() ? "Y" : "N");

		// end, if an incomplete message is found
		if (!msg.complete())
			break;

		// at this point, any failure means we can delete the current message
		it++;

		// Scan for message start
		if (memcmp(msg.hdr.pchMessageStart, Params().MessageStart(), MESSAGE_START_SIZE) != 0) {
			LogPrintf("\n\nPROCESSMESSAGE: INVALID MESSAGESTART\n\n");
			fOk = false;
			break;
		}

		// Read header
		CMessageHeader& hdr = msg.hdr;
		if (!hdr.IsValid())
		{
			LogPrintf("\n\nPROCESSMESSAGE: ERRORS IN HEADER %s\n\n\n", hdr.GetCommand());
			continue;
		}
		string strCommand = hdr.GetCommand();

		// Message size
		unsigned int nMessageSize = hdr.nMessageSize;

		// Checksum
		CDataStream& vRecv = msg.vRecv;
		uint256 hash = Hash(vRecv.begin(), vRecv.begin() + nMessageSize);
		unsigned int nChecksum = 0;
		memcpy(&nChecksum, &hash, sizeof(nChecksum));
		if (nChecksum != hdr.nChecksum)
		{
			LogPrintf("ProcessMessages(%s, %u bytes) : CHECKSUM ERROR nChecksum=%08x hdr.nChecksum=%08x\n",
			   strCommand, nMessageSize, nChecksum, hdr.nChecksum);
			continue;
		}

		// Process message
		bool fRet = false;
		try
		{
			fRet = processMessage(pfrom, vRecv);
			boost::this_thread::interruption_point();
		}
		catch (std::ios_base::failure& e)
		{
			pfrom->PushMessage("reject", strCommand, REJECT_MALFORMED, string("error parsing message"));
			if (strstr(e.what(), "end of data"))
			{
				// Allow exceptions from under-length message on vRecv
				LogPrintf("ProcessMessages(%s, %u bytes) : Exception '%s' caught, normally caused by a message being shorter than its stated length\n", strCommand, nMessageSize, e.what());
			}
			else if (strstr(e.what(), "size too large"))
			{
				// Allow exceptions from over-long size
				LogPrintf("ProcessMessages(%s, %u bytes) : Exception '%s' caught\n", strCommand, nMessageSize, e.what());
			}
			else
			{
				PrintExceptionContinue(&e, "ProcessMessages()");
			}
		}
		catch (boost::thread_interrupted) {
			throw;
		}
		catch (std::exception& e) {
			PrintExceptionContinue(&e, "ProcessMessages()");
		} catch (...) {
			PrintExceptionContinue(NULL, "ProcessMessages()");
		}

		if (!fRet)
			LogPrintf("ProcessMessage(%s, %u bytes) FAILED\n", strCommand, nMessageSize);

		break;
	}

	// In case the connection got shut down, its receive buffer was wiped
	if (!pfrom->fDisconnect)
		pfrom->vRecvMsg.erase(pfrom->vRecvMsg.begin(), it);

	return fOk;
}


bool
CManageNetwork::sendMessages(CNode* pto, bool fSendTrickle)
{
	std::vector< CMessage > messages;
	CNodesManager::getInstance()->getMessagesForNode( pto, messages );
	pto->PushMessage("", messages);
	/*
	{
		// Don't send anything until we get their version message
		if (pto->nVersion == 0)
			return true;

		//
		// Message: ping
		//
		bool pingSend = false;
		if (pto->fPingQueued) {
			// RPC ping request by user
			pingSend = true;
		}
		if (pto->nLastSend && GetTime() - pto->nLastSend > 30 * 60 && pto->vSendMsg.empty()) {
			// Ping automatically sent as a keepalive
			pingSend = true;
		}
		if (pingSend) {
			uint64_t nonce = 0;
			while (nonce == 0) {
				RAND_bytes((unsigned char*)&nonce, sizeof(nonce));
			}
			pto->nPingNonceSent = nonce;
			pto->fPingQueued = false;
			if (pto->nVersion > BIP0031_VERSION) {
				// Take timestamp as close as possible before transmitting ping
				pto->nPingUsecStart = GetTimeMicros();
				pto->PushMessage("ping", nonce);
			} else {
				// Peer is too old to support ping command with nonce, pong will never arrive, disable timing
				pto->nPingUsecStart = 0;
				pto->PushMessage("ping");
			}
		}

		// Address refresh broadcast
		static int64_t nLastRebroadcast;
		if (!IsInitialBlockDownload() && (GetTime() - nLastRebroadcast > 24 * 60 * 60))
		{
			{
				LOCK(cs_vNodes);
				BOOST_FOREACH(CNode* pnode, vNodes)
				{
					// Periodically clear setAddrKnown to allow refresh broadcasts
					if (nLastRebroadcast)
						pnode->setAddrKnown.clear();
				}
			}
			nLastRebroadcast = GetTime();
		}

		//
		// Message: addr
		//
		if (fSendTrickle)
		{
			vector<CAddress> vAddr;
			vAddr.reserve(pto->vAddrToSend.size());
			BOOST_FOREACH(const CAddress& addr, pto->vAddrToSend)
			{
				// returns true if wasn't already contained in the set
				if (pto->setAddrKnown.insert(addr).second)
				{
					vAddr.push_back(addr);
					// receiver rejects addr messages larger than 1000
					if (vAddr.size() >= 1000)
					{
						pto->PushMessage("addr", vAddr);
						vAddr.clear();
					}
				}
			}
			pto->vAddrToSend.clear();
			if (!vAddr.empty())
				pto->PushMessage("addr", vAddr);
		}

		TRY_LOCK(cs_main, lockMain);
		if (!lockMain)
			return true;

		CNodeState &state = *State(pto->GetId());
		if (state.fShouldBan) {
			if (pto->addr.IsLocal())
				LogPrintf("Warning: not banning local node %s!\n", pto->addr.ToString());
			else {
				pto->fDisconnect = true;
				CNode::Ban(pto->addr);
			}
			state.fShouldBan = false;
		}

		BOOST_FOREACH(const CBlockReject& reject, state.rejects)
			pto->PushMessage("reject", (string)"block", reject.chRejectCode, reject.strRejectReason, reject.hashBlock);
		state.rejects.clear();


	 //

		// Start block sync
		if (pto->fStartSync && !fImporting && !fReindex) {
			pto->fStartSync = false;
			CBloomFilter bloomFilter =  CBloomFilter(10, 0.000001, 0, BLOOM_UPDATE_P2PUBKEY_ONLY);
			bloomFilter.insert(ParseHex(Params().getOriginAddressAsString()));
			pto->PushMessage("filterload", bloomFilter);

			PushGetHeaders(pto, chainActive.Tip(), uint256(0));
		}

		//
		// Message: getdata
		//
		vector<CInv> vGetData;
		int64_t nNow = GetTime() * 1000000;
		while (!pto->mapAskFor.empty() && (*pto->mapAskFor.begin()).first <= nNow)
		{
			const CInv& inv = (*pto->mapAskFor.begin()).second;
			if (!AlreadyHave(inv))
			{
				if (fDebug)
					LogPrint("net", "sending getdata: %s\n", inv.ToString());
				vGetData.push_back(inv);
				if (vGetData.size() >= 1000)
				{
					pto->PushMessage("getdata", vGetData);
					vGetData.clear();
				}
			}
			pto->mapAskFor.erase(pto->mapAskFor.begin());
		}
		if (!vGetData.empty())
			pto->PushMessage("getdata", vGetData);

	}*/
	return true;
}


}
