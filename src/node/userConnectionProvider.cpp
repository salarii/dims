#include "userConnectionProvider.h"
#include "dummyMedium.h"


namespace node
{

CUserConnectionProvider::CUserConnectionProvider()
{
	m_dummyMedium = new CDummyMedium();
}

CMedium *
CUserConnectionProvider::provideConnection( RequestKind::Enum const _actionKind )
{
	return _actionKind == RequestKind::NetworkInfo ? m_dummyMedium : 0;
}

std::list< CMedium *>
CUserConnectionProvider::provideConnection( RequestKind::Enum const _actionKind, unsigned _requestedConnectionNumber )
{
	return std::list< CMedium *>();
}

}