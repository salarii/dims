#include "settingsConnectionProvider.h"
#include "settingsMedium.h"


namespace client
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
CSettingsConnectionProvider::provideConnection( common::CMediumFilter< NodeResponses > const & _mediumFilter )
{
	std::list< common::CMedium< NodeResponses > *> mediums;
	if( common::RequestKind::NetworkInfo == _mediumFilter.m_mediumClass || common::RequestKind::Seed == _mediumFilter.m_mediumClass )// temporary???
	{
		mediums.push_back( m_settingsMedium );
	}
	return mediums;
}

}
