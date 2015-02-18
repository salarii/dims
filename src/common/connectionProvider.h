// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECTION_PROVIDER_H
#define CONNECTION_PROVIDER_H

#include <list>

#include "request.h"
#include "filters.h"

namespace common
{
template < class _RequestResponses >
class CMedium;
//?? one provider  for  every executable file??? right now  there is many of  them
template < class _RequestResponses >
class CConnectionProvider
{
public:
	virtual std::list< CMedium< _RequestResponses > *> provideConnection( CMediumFilter< _RequestResponses > const & ) = 0;
};

}


#endif // CONNECTION_PROVIDER_H
