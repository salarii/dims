// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SEED_SIMPLIFIED_NETWORK_MANAGER_H
#define SEED_SIMPLIFIED_NETWORK_MANAGER_H


#include <list>
#include <vector>

#include <boost/thread.hpp>
#include <boost/signals2.hpp>
#include "seedNetbase.h"

#include "sync.h"

namespace seed
{

class CAddrDb;

class CSimplifiedNetworkManager
{
public:
	CSimplifiedNetworkManager( CAddrDb & _db );

	bool connectToNetwork();


	enum BindFlags {
		BF_NONE         = 0,
		BF_EXPLICIT     = (1U << 0),
		BF_REPORT_ERROR = (1U << 1)
	};

	static void* runSocketThread(void *context)
	{
		((CSimplifiedNetworkManager *)context)->threadSocketHandler();
	}
private:
	void threadSocketHandler();

	bool bind(const CService &addr, unsigned int flags);

	bool bindListenPort(const CService &addrBind, std::string& strError);
private:
	std::vector<SOCKET> m_listenSocket;

	uint64_t nLocalServices;

	unsigned int m_maxConnections;

	CAddrDb & db;

	pthread_t threadManageSockets;
};

}

#endif // SEED_SIMPLIFIED_NETWORK_MANAGER_H
