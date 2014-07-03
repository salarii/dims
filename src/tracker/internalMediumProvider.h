// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef INTERNAL_MEDIUM_PROVIDER_H
#define INTERNAL_MEDIUM_PROVIDER_H

#include "common/connectionProvider.h"
#include "common/medium.h"
namespace tracker
{
// singleton??

class CInternalMediumProvider : public  common::CConnectionProvider< TrackerResponses >
{
public:
	CInternalMediumProvider();
	virtual std::list< common::CMedium< TrackerResponses > *> provideConnection( int const _actionKind, unsigned _requestedConnectionNumber = -1 );
private:
	std::list< common::CMedium< TrackerResponses > *> m_mediums;
};

}

#endif // INTERNAL_MEDIUM_PROVIDER_H
