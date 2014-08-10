#ifndef FILTERS_H
#define FILTERS_H

#include "connectionProvider.h"
#include "support.h"

#include "boost/foreach.hpp"
#include "tracker/configureTrackerActionHandler.h"
#include "node/configureNodeActionHadler.h"
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

class CSettingsMedium;
class CTrackerLocalRanking;

}

namespace monitor
{

}

namespace common
{

template < class ResponseType >
class CNodesManager;

struct Dummy0; struct Dummy1; struct Dummy2; struct Dummy3; struct Dummy4; struct Dummy5;

template < class _RequestResponses, class _V0 = Dummy0, class _V1 = Dummy1, class _V2 = Dummy2, class _V3 = Dummy3, class _V4 = Dummy4, class _V5 = Dummy5 >
struct CMediumFilterBase
{
	virtual std::list< CMedium< _RequestResponses > *> getMediums( _V0 * _v0 )const{ return std::list< CMedium< _RequestResponses > *>(); }
	virtual std::list< CMedium< _RequestResponses > *> getMediums( _V1 * _v1 )const{ return std::list< CMedium< _RequestResponses > *>(); }
	virtual std::list< CMedium< _RequestResponses > *> getMediums( _V2 * _v2 )const{ return std::list< CMedium< _RequestResponses > *>(); }
	virtual std::list< CMedium< _RequestResponses > *> getMediums( _V3 * _v3 )const{ return std::list< CMedium< _RequestResponses > *>(); }
	virtual std::list< CMedium< _RequestResponses > *> getMediums( _V4 * _v4 )const{ return std::list< CMedium< _RequestResponses > *>(); }
	virtual std::list< CMedium< _RequestResponses > *> getMediums( _V5 * _v5 )const{ return std::list< CMedium< _RequestResponses > *>(); }
};

template < class _RequestResponses >
struct CMediumFilter : public CMediumFilterBase< _RequestResponses >
{
};

template <>
struct CMediumFilter< tracker::TrackerResponses > : public CMediumFilterBase< tracker::TrackerResponses, common::CNodesManager< tracker::TrackerResponses >, tracker::CInternalMediumProvider >
{
};

template <>
struct CMediumFilter< client::NodeResponses > : public CMediumFilterBase< client::NodeResponses, client::CSettingsMedium, common::CNodesManager< client::NodeResponses > >
{
};

template <>
struct CMediumFilter< seed::SeedResponses > : public CMediumFilterBase< seed::SeedResponses, common::CNodesManager< seed::SeedResponses > >
{
};



}

#endif // FILTERS_H
