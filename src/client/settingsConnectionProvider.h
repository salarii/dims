// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SETTINGS_CONNECTION_PROVIDER_H
#define SETTINGS_CONNECTION_PROVIDER_H

#include "common/connectionProvider.h"

#include "client/struct.h"

namespace client
{
class CDefaultMedium;
// stupid  name
class CSettingsConnectionProvider : public common::CConnectionProvider
{
public:
	virtual std::list< common::CMedium *> provideConnection( common::CMediumFilter const & _mediumFilter );

	std::list< common::CMedium *> getMediumByClass( ClientMediums::Enum _requestKind );

	static CSettingsConnectionProvider* getInstance();

	void setTrackerIp( std::string const & _ip );
private:

	CSettingsConnectionProvider();
private:
	static CSettingsConnectionProvider * ms_instance;

	CDefaultMedium * m_settingsMedium;
};

}

#endif // SETTINGS_CONNECTION_PROVIDER_H
