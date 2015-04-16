// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECTION_PROVIDER_H
#define CONNECTION_PROVIDER_H

#include <list>

#include "request.h"
#include "filters.h"
#include "types.h"

namespace common
{

template < class _Type >
class CConnectionProvider
{
public:
	virtual std::list< typename _Type::Medium *> provideConnection( typename _Type::Filter const & ) = 0;

	virtual ~CConnectionProvider(){};
};

}


#endif // CONNECTION_PROVIDER_H
