// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECTION_PROVIDER_H
#define CONNECTION_PROVIDER_H

#include <list>

#include "common/request.h"
#include "common/filters.h"

namespace common
{

class CMedium;
class CMediumFilter;

class CConnectionProvider
{
public:
	virtual std::list< CMedium *> provideConnection( CMediumFilter const & ) = 0;

	virtual ~CConnectionProvider(){};
};

}


#endif // CONNECTION_PROVIDER_H
