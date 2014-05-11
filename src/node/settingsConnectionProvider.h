// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SETTINGS_CONNECTION_PROVIDER_H
#define SETTINGS_CONNECTION_PROVIDER_H

#include "common/connectionProvider.h"
#include "configureNodeActionHadler.h"

namespace node
{
class CSettingsMedium;

class CSettingsConnectionProvider : public common::CConnectionProvider< NodeResponses >
{
public:
	virtual std::list< common::CMedium< NodeResponses > *> provideConnection( int const _actionKind, unsigned _requestedConnectionNumber = -1 );

	static CSettingsConnectionProvider* getInstance();

	void setTrackerIp( std::string const & _ip );
private:

	CSettingsConnectionProvider();
private:
	static CSettingsConnectionProvider * ms_instance;

	CSettingsMedium * m_settingsMedium;
};

}

#endif // SETTINGS_CONNECTION_PROVIDER_H
