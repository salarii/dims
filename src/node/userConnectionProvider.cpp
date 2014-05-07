#include "userConnectionProvider.h"
#include "dummyMedium.h"


namespace node
{

CUserConnectionProvider::CUserConnectionProvider()
{
	m_dummyMedium = new CDummyMedium();
}

std::list< common::CMedium< NodeResponses > *>
CUserConnectionProvider::provideConnection( int const _actionKind, unsigned _requestedConnectionNumber )
{
	return std::list< common::CMedium< NodeResponses > *>();
}

}
