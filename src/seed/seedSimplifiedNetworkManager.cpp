// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "seedSimplifiedNetworkManager.h"

#include "seedDb.h"

#include "util.h"

#include "boost/foreach.hpp"

namespace seed
{

static const int MAX_OUTBOUND_CONNECTIONS = 64;
extern pthread_mutex_t nodesLock;

CSimplifiedNetworkManager::CSimplifiedNetworkManager( CAddrDb & _db )
	: nLocalServices( NODE_NETWORK )
	, db( _db )
{
}

bool
CSimplifiedNetworkManager::connectToNetwork()
{
	bool bound = false;
	struct in_addr inaddr_any;
	inaddr_any.s_addr = INADDR_ANY;

#ifdef USE_IPV6
	bound = bind(CService(in6addr_any, GetDefaultPort()), BF_NONE);
#endif
	bound = bind(CService(inaddr_any, GetDefaultPort()), BF_REPORT_ERROR );

	// Send and receive from sockets, accept connections
	pthread_create(&threadManageSockets, NULL, &CSimplifiedNetworkManager::runSocketThread, this);

	return bound;
}

void
CSimplifiedNetworkManager::threadSocketHandler()
{
	unsigned int nPrevNodeCount = 0;
	while (true)
	{
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
		pthread_mutex_lock(&nodesLock);

		BOOST_FOREACH(SOCKET hListenSocket, m_listenSocket) {
			FD_SET(hListenSocket, &fdsetRecv);
			hSocketMax = max(hSocketMax, hListenSocket);
			have_fds = true;
		}


		int nSelect = select(have_fds ? hSocketMax + 1 : 0,
			&fdsetRecv, &fdsetSend, &fdsetError, &timeout);


		if (nSelect == SOCKET_ERROR)
		{
			if (have_fds)
			{
				int nErr = WSAGetLastError();

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
					nInbound++;
				}

				if (hSocket == INVALID_SOCKET)
				{
					int nErr = WSAGetLastError();
					if (nErr != WSAEWOULDBLOCK)
						LogPrintf("socket error accept failed: %d\n", nErr);
				}
				else if (nInbound >= MAX_OUTBOUND_CONNECTIONS)
				{
					closesocket(hSocket);
				}
				else
				{
					db.Add(addr);
					closesocket(hSocket);
				}
			}
			  pthread_mutex_unlock(&nodesLock);
		MilliSleep(100);
	}
}

bool
CSimplifiedNetworkManager::bind(const CService &addr, unsigned int flags)
{
	std::string strError;
	if (!bindListenPort(addr, strError)) {
		if (flags & BF_REPORT_ERROR)
			return false;
		return false;
	}
	return true;
}

bool
CSimplifiedNetworkManager::bindListenPort(const CService &addrBind, string& strError)
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

	return true;
}


}

