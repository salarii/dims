// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ERROR_MEDIUM_PROVIDER_H
#define ERROR_MEDIUM_PROVIDER_H

#include "common/medium.h"
#include "common/connectionProvider.h"

#include "configureClientActionHadler.h"

namespace client
{

class CErrorMediumProvider : public common::CConnectionProvider< common::CClientTypes >
{
public:
	virtual std::list< common::CClientBaseMedium *> provideConnection( common::CClientMediumFilter const & _mediumFilter );

	std::list< common::CClientBaseMedium *> getErrorMedium();

	static CErrorMediumProvider* getInstance();
private:

	CErrorMediumProvider();
private:
	static CErrorMediumProvider * ms_instance;
};

}

#endif // ERROR_MEDIUM_PROVIDER_H
