// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "seedProvider.h"

#include <set>
#include <QString>
#include "common/commonResponses.h"

namespace client
{

CSeedProvider * CSeedProvider::ms_instance = NULL;


std::list< common::CMedium< NodeResponses > *>
CSeedProvider::provideConnection( common::CMediumFilter< NodeResponses > const & _mediumFilter )
{
	return std::list< common::CMedium< NodeResponses > *>();
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
