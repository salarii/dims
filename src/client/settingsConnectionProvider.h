// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SETTINGS_CONNECTION_PROVIDER_H
#define SETTINGS_CONNECTION_PROVIDER_H

#include "common/connectionProvider.h"
#include "configureClientActionHadler.h"

namespace client
{
class CDefaultMedium;

class CSettingsConnectionProvider : public common::CConnectionProvider< ClientResponses >
{
public:
	virtual std::list< common::CMedium< ClientResponses > *> provideConnection( common::CMediumFilter< ClientResponses > const & _mediumFilter );

	std::list< common::CMedium< ClientResponses > *> getMediumByClass( common::RequestKind::Enum _requestKind );

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
