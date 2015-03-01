// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
}

CSettingsConnectionProvider::CSettingsConnectionProvider()
{
	m_settingsMedium = client::CDefaultMedium::getInstance();
}

std::list< common::CMedium< ClientResponses > *>
CSettingsConnectionProvider::provideConnection( common::CMediumFilter< ClientResponses > const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

std::list< common::CMedium< ClientResponses > *>
CSettingsConnectionProvider::getMediumByClass( common::RequestKind::Enum _requestKind )
{
	std::list< common::CMedium< ClientResponses > *> mediums;
	if( common::RequestKind::NetworkInfo == _requestKind || common::RequestKind::Seed == _requestKind )// temporary???
	{
		mediums.push_back( m_settingsMedium );
	}

	return mediums;
}


}
