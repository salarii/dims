#include "seedProvider.h"

#include <set>
#include <QString>

namespace node
{

CSeedProvider * CSeedProvider::ms_instance = NULL;

CNetworkClient * 
CSeedProvider::provideConnection( RequestKind::Enum const _actionKind )
{
}

std::list< CNetworkClient *>
CSeedProvider::provideConnection( RequestKind::Enum const _actionKind, unsigned _requestedConnectionNumber )
{
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