#include "manageNetwork.h"

namespace self
{

CSemaphore * CManageNetwork::ms_semOutbound = NULL

unsigned int CManageNetwork::m_maxConnections = 16; // this  should be read from network parameters

CManageNetwork::CManageNetwork()
{
}

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

void
CManageNetwork::connectToNetwork()
{
	if (ms_semOutbound == NULL) {
		// initialize semaphore
		ms_semOutbound = new CSemaphore(m_maxConnections);
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
	threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "net", &threadSocketHandler));

	// Initiate outbound connections from -addnode
	threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "addcon", &threadOpenAddedConnections));

	// Initiate outbound connections
	threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "opencon", &threadOpenConnections));

	// Process messages
	threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "msghand", &ThreadMessageHandler));

	// Dump network addresses
//	threadGroup.create_thread(boost::bind(&LoopForever<void (*)()>, "dumpaddr", &DumpAddresses, DUMP_ADDRESSES_INTERVAL * 1000));
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
	if (!IsLimited(NET_IPV4))
		threadGroup.create_thread(boost::bind(&TraceThread<void (*)()>, "ext-ip", &ThreadGetMyExternalIP));
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
	return AddLocal(CService(addr, GetListenPort()), nScore);
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
					|| !m_nodesManager->isNodeHonest( pnode ))
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
				else if (m_nodesManager->isBanned( addr ))
				{
					LogPrintf("connection from %s dropped (banned)\n", addr.ToString());
					closesocket(hSocket);
				}
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
				OpenNetworkConnection(addr, &grant, strAddNode.c_str());
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
			if(Lookup(strAddNode.c_str(), vservNode, Params().GetDefaultPort(), fNameLookup, 0))
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
			OpenNetworkConnection(CAddress(vserv[i % vserv.size()]), &grant);
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
			ProcessOneShot();
			BOOST_FOREACH(string strAddr, mapMultiArgs["-connect"])
			{
				CAddress addr;
				OpenNetworkConnection(addr, NULL, strAddr.c_str());
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
			if (addr.GetPort() != Params().GetDefaultPort() && nTries < 50)
				continue;

			addrConnect = addr;
			break;
		}

		if (addrConnect.IsValid())
			OpenNetworkConnection(addrConnect, &grant);
	}
}

bool
CManageNetwork::::OpenNetworkConnection(const CAddress& addrConnect, CSemaphoreGrant *grantOutbound, const char *strDest, bool fOneShot)
{
	//
	// Initiate outbound network connection
	//
	boost::this_thread::interruption_point();
	if (!strDest)
		if (IsLocal(addrConnect) ||
			FindNode((CNetAddr)addrConnect) || CNode::IsBanned(addrConnect) ||
			FindNode(addrConnect.ToStringIPPort().c_str()))
			return false;
	if (strDest && FindNode(strDest))
		return false;

	CNode* pnode = ConnectNode(addrConnect, strDest);
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
					if (!g_signals.ProcessMessages(pnode))
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

			// Send messages
			{
				TRY_LOCK(pnode->cs_vSend, lockSend);
				if (lockSend)
					g_signals.SendMessages(pnode, pnode == pnodeTrickle);
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

}