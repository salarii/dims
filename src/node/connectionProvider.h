// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECTION_PROVIDER_H
#define CONNECTION_PROVIDER_H

#include <list>

#include "common/request.h"

namespace node
{

class CMedium;

class CConnectionProvider
{
public:
	virtual CMedium * provideConnection( common::RequestKind::Enum const _actionKind ) = 0;

	virtual std::list< CMedium *> provideConnection( common::RequestKind::Enum const _actionKind, unsigned _requestedConnectionNumber ) = 0;
};

}


#endif // CONNECTION_PROVIDER_H
