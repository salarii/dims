// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NO_MEDIUM_HANDLING_H
#define NO_MEDIUM_HANDLING_H

#include "common/connectionProvider.h"
#include "common/medium.h"
#include "common/mediumKinds.h"

namespace monitor
{

class CErrorMediumProvider : public  common::CConnectionProvider< common::CMonitorMediumFilter >
{
public:
	std::list< common::CMonitorBaseMedium *> provideConnection( common::CMonitorMediumFilter const & _filter );

	static CErrorMediumProvider* getInstance( );
private:
	CErrorMediumProvider();
private:
	mutable boost::mutex m_mutex;

	static CErrorMediumProvider * ms_instance;

	std::list< common::CMonitorBaseMedium *> m_mediums;
};

}

#endif // NO_MEDIUM_HANDLING_H
