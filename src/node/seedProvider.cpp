#include "seedProvider.h"

#include <set>
#include <QString>

namespace node
{

CSeedProvider * CSeedProvider::ms_instance = NULL;

CMedium * 
CSeedProvider::provideConnection( RequestKind::Enum const _actionKind )
{
	return 0;
}

std::list< CMedium *>
CSeedProvider::provideConnection( RequestKind::Enum const _actionKind, unsigned _requestedConnectionNumber )
{
	return std::list< CMedium *>();
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