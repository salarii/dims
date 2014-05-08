#include "userConnectionProvider.h"
#include "dummyMedium.h"


namespace node
{
// in the future here  mediums from  user settings  should appear for now it  is mostly dummy stuff
CUserConnectionProvider::CUserConnectionProvider()
{
	m_dummyMedium = new CDummyMedium();
}

std::list< common::CMedium< NodeResponses > *>
CUserConnectionProvider::provideConnection( int const _actionKind, unsigned _requestedConnectionNumber )
{
	std::list< common::CMedium< NodeResponses > *> mediums;
	if( common::RequestKind::NetworkInfo == _actionKind )
	{
		mediums.push_back( m_dummyMedium );
	}
	return mediums;
}

}
