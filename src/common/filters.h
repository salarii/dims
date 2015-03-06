// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FILTERS_H
#define FILTERS_H

#include "connectionProvider.h"
#include "support.h"

#include "boost/foreach.hpp"
#include "tracker/configureTrackerActionHandler.h"
#include "client/configureClientActionHadler.h"
#include "monitor/configureMonitorActionHandler.h"
#include "seed/configureSeedActionHandler.h"

namespace tracker
{

class CInternalMediumProvider;

}

namespace seed
{

class CSeedNodesManager;

}

namespace client
{
class CSettingsConnectionProvider;

class CTrackerLocalRanking;

class CLocalServer;

class CErrorMediumProvider;
}

namespace monitor
{
class CInternalMediumProvider;
}

namespace common
{
class CClientBaseMedium;

class CMonitorBaseMedium;

class CSeedBaseMedium;

class CTrackerBaseMedium;

template < class ResponseType >
class CNodesManager;

struct Dummy0; struct Dummy1; struct Dummy2; struct Dummy3; struct Dummy4; struct Dummy5;struct Dummy6; struct Dummy7;

template < class _Medium, class _V0 = Dummy0, class _V1 = Dummy1, class _V2 = Dummy2, class _V3 = Dummy3, class _V4 = Dummy4, class _V5 = Dummy5, class _V6 = Dummy6, class _V7 = Dummy7>
struct CMediumFilterBase
{
	typedef _Medium Medium;

	virtual std::list< _Medium *> getMediums( _V0 * _v0 )const{ return std::list< _Medium *>(); }
	virtual std::list< _Medium *> getMediums( _V1 * _v1 )const{ return std::list< _Medium *>(); }
	virtual std::list< _Medium *> getMediums( _V2 * _v2 )const{ return std::list< _Medium *>(); }
	virtual std::list< _Medium *> getMediums( _V3 * _v3 )const{ return std::list< _Medium *>(); }
	virtual std::list< _Medium *> getMediums( _V4 * _v4 )const{ return std::list< _Medium *>(); }
	virtual std::list< _Medium *> getMediums( _V5 * _v5 )const{ return std::list< _Medium *>(); }
	virtual std::list< _Medium *> getMediums( _V6 * _v6 )const{ return std::list< _Medium *>(); }
	virtual std::list< _Medium *> getMediums( _V7 * _v7 )const{ return std::list< _Medium *>(); }
};

struct CTrackerMediumFilter : public CMediumFilterBase< CTrackerBaseMedium, common::CNodesManager< tracker::TrackerResponses >, tracker::CInternalMediumProvider >
{
};

struct CClientMediumFilter : public CMediumFilterBase< CClientBaseMedium, client::CSettingsConnectionProvider, client::CTrackerLocalRanking, client::CLocalServer, client::CErrorMediumProvider >
{
};

struct CSeedMediumFilter : public CMediumFilterBase< CSeedBaseMedium, seed::CSeedNodesManager, common::CNodesManager< seed::SeedResponses > >
{
};

struct CMonitorMediumFilter : public CMediumFilterBase< CMonitorBaseMedium, common::CNodesManager< monitor::MonitorResponses >, monitor::CInternalMediumProvider >
{
};

}

#endif // FILTERS_H
