// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FILTERS_H
#define FILTERS_H

#include "connectionProvider.h"
#include "support.h"

#include "boost/foreach.hpp"

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
class CReputationTracker;
class CInternalMediumProvider;
}

namespace common
{

class CMedium;

class CNodesManager;

struct CMediumFilter
{
	virtual std::list< CMedium *> getMediums( common::CNodesManager * _provider )const{ return std::list< CMedium *>(); }
	virtual std::list< CMedium *> getMediums( tracker::CInternalMediumProvider * _provider )const{ return std::list< CMedium *>(); }
	virtual std::list< CMedium *> getMediums( client::CSettingsConnectionProvider * _provider )const{ return std::list< CMedium *>(); }
	virtual std::list< CMedium *> getMediums( client::CTrackerLocalRanking * _provider )const{ return std::list< CMedium *>(); }
	virtual std::list< CMedium *> getMediums( client::CLocalServer * _provider )const{ return std::list< CMedium *>(); }
	virtual std::list< CMedium *> getMediums( client::CErrorMediumProvider * _provider )const{ return std::list< CMedium *>(); }
	virtual std::list< CMedium *> getMediums( seed::CSeedNodesManager * _provider )const{ return std::list< CMedium *>(); }
	virtual std::list< CMedium *> getMediums( monitor::CInternalMediumProvider * _provider )const{ return std::list< CMedium *>(); }
	virtual std::list< CMedium *> getMediums( monitor::CReputationTracker * _provider )const{ return std::list< CMedium *>(); }

	virtual ~CMediumFilter(){}
};

}

#endif // FILTERS_H
