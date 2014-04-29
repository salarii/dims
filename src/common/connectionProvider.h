// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECTION_PROVIDER_H
#define CONNECTION_PROVIDER_H

#include <list>

#include "request.h"

namespace common
{
template < class _RequestResponses >
class CMedium;

template < class _RequestResponses >
class CConnectionProvider
{
public:
	virtual std::list< CMedium< _RequestResponses > *> provideConnection( int const _actionKind, unsigned _requestedConnectionNumber = -1 ) = 0;
};

}


#endif // CONNECTION_PROVIDER_H
