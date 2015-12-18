// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "seedProvider.h"

#include <set>
#include <QString>
#include "common/responses.h"

namespace client
{

CSeedProvider * CSeedProvider::ms_instance = NULL;


std::list< common::CMedium *>
CSeedProvider::provideConnection( common::CMediumFilter const & _mediumFilter )
{
	return std::list< common::CMedium *>();
}

CSeedProvider::~CSeedProvider()
{
}

CSeedProvider *
CSeedProvider::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CSeedProvider();
	};
	return ms_instance;
}

CSeedProvider::CSeedProvider()
{
}

}
