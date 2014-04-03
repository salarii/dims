// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECTION_PROVIDER_H
#define CONNECTION_PROVIDER_H

#include <list>

namespace node
{

class CNetworkClient;

struct RequestKind
{
	enum Enum
	{
		Info = 0,
		Transaction,
		TransactionStatus,
		Seed
	};
};

class CConnectionProvider
{
public:
	virtual CNetworkClient * provideConnection( RequestKind::Enum const _actionKind ) = 0;

	virtual std::list< CNetworkClient *> provideConnection( RequestKind::Enum const _actionKind, unsigned _requestedConnectionNumber ) = 0;
};

}


#endif // CONNECTION_PROVIDER_H