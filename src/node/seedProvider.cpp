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
