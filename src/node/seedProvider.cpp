#include "seedProvider.h"

#include <set>
#include <QString>
#include "common/trackerResponses.h"

namespace node
{

CSeedProvider * CSeedProvider::ms_instance = NULL;


std::list< common::CMedium< NodeResponses > *>
CSeedProvider::provideConnection( int const _actionKind, unsigned _requestedConnectionNumber )
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
