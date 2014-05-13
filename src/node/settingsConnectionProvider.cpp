#include "settingsConnectionProvider.h"
#include "settingsMedium.h"


namespace node
{

CSettingsConnectionProvider * CSettingsConnectionProvider::ms_instance = NULL;

CSettingsConnectionProvider*
CSettingsConnectionProvider::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CSettingsConnectionProvider();
	};
	return ms_instance;
}

void
CSettingsConnectionProvider::setTrackerIp( std::string const & _ip )
{
	m_settingsMedium->setTrackerIp( _ip );
}


// in the future here  mediums from  user settings  should appear for now it  is mostly dummy stuff
CSettingsConnectionProvider::CSettingsConnectionProvider()
{
	m_settingsMedium = new CSettingsMedium();
}

std::list< common::CMedium< NodeResponses > *>
CSettingsConnectionProvider::provideConnection( int const _actionKind, unsigned _requestedConnectionNumber )
{
	std::list< common::CMedium< NodeResponses > *> mediums;
	if( common::RequestKind::NetworkInfo == _actionKind )
	{
		mediums.push_back( m_settingsMedium );
	}
	return mediums;
}

}