// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_NODE_MEDIUM_H
#define TRACKER_NODE_MEDIUM_H

#include "common/nodeMedium.h"
#include "common/communicationProtocol.h"

#include <boost/variant.hpp>

namespace monitor
{

class CRegistrationTerms;

class CInfoRequest;

class CMonitorNodeMedium : public common::CNodeMedium
{
public:
	CMonitorNodeMedium( common::CSelfNode * _selfNode ):common::CNodeMedium( _selfNode ){};

	void add( CRegistrationTerms const * _request );

	void add( CInfoRequest const * _request );
private:
};

}

#endif // TRACKER_NODE_MEDIUM_H
