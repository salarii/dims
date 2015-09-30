// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "settingsConnectionProvider.h"
#include "settingsMedium.h"

#include "common/timeMedium.h"

#include "client/filters.h"

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

std::list< common::CMedium *>
CSettingsConnectionProvider::provideConnection( common::CMediumFilter const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

std::list< common::CMedium *>
CSettingsConnectionProvider::getMediumByClass( ClientMediums::Enum _requestKind )
{
	std::list< common::CMedium *> mediums;
	if( ClientMediums::NetworkInfo == _requestKind || ClientMediums::Seed == _requestKind )// temporary???
	{
		mediums.push_back( m_settingsMedium );
	}
	else if ( ClientMediums::Time == _requestKind )
	{
		mediums.push_back( common::CTimeMedium::getInstance() );
	}

	return mediums;
}

}
