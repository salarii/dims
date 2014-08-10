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

class CTrackerNodesManager;
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

struct Dummy0; struct Dummy1; struct Dummy2; struct Dummy3; struct Dummy4; struct Dummy5;

template < class _RequestResponses, class _V0 = Dummy0, class _V1 = Dummy1, class _V2 = Dummy2, class _V3 = Dummy3, class _V4 = Dummy4, class _V5 = Dummy5 >
struct CMediumFilterBase
{
	virtual std::list< CMedium< _RequestResponses > *> filterFrom( _V0 * _v0 ){ return std::list< CMedium< _RequestResponses > *>(); }
	virtual std::list< CMedium< _RequestResponses > *> filterFrom( _V1 * _v1 ){ return std::list< CMedium< _RequestResponses > *>(); }
	virtual std::list< CMedium< _RequestResponses > *> filterFrom( _V2 * _v2 ){ return std::list< CMedium< _RequestResponses > *>(); }
	virtual std::list< CMedium< _RequestResponses > *> filterFrom( _V3 * _v3 ){ return std::list< CMedium< _RequestResponses > *>(); }
	virtual std::list< CMedium< _RequestResponses > *> filterFrom( _V4 * _v4 ){ return std::list< CMedium< _RequestResponses > *>(); }
	virtual std::list< CMedium< _RequestResponses > *> filterFrom( _V5 * _v5 ){ return std::list< CMedium< _RequestResponses > *>(); }
};

template < class _RequestResponses >
struct CMediumFilter : public CMediumFilterBase
{
};

template < tracker::TrackerResponses >
struct CMediumFilter : public CMediumFilterBase< tracker::TrackerResponses, CTrackerNodesManager, CInternalMediumProvider >
{
};

template < client::NodeResponses >
struct CMediumFilter : public CMediumFilterBase< client::NodeResponses, CSettingsMedium, CTrackerLocalRanking >
{
};

template < seed::SeedResponses >
struct CMediumFilter : public CMediumFilterBase< seed::SeedResponses, CSeedNodesManager >
{
};
/*
 : public CMediumFilterBase

	template < class _RequestResponses >
	struct CAcceptFilterByPtr : public CAcceptFilter< _RequestResponses >
	{
		CAcceptFilterByPtr( unsigned long long _ptr )
			:m_ptr( _ptr )
		{
		}

		std::list< CMedium< _RequestResponses > *> accepted( std::list< CMedium< _RequestResponses > *> const & _candidates )
		{
			std::list< CMedium< _RequestResponses > *> acceptedMedium;

			BOOST_FOREACH( CMedium< _RequestResponses > * medium, _candidates )
			{
				if ( convertToInt( medium ) == m_ptr )
				{
					acceptedMedium.push_back( medium );
				}
			}
		}

		unsigned long long m_ptr;
	};

	template < class _RequestResponses >
	CMediumFilter< _RequestResponses > *
	createFilterWithPtr( int _mediumClass, int _mediumNumber, unsigned long long _ptr )
	{
		return new common::CMediumFilter< _RequestResponses >( _mediumClass, _mediumNumber, new common::CAcceptFilterByPtr< _RequestResponses >( _ptr ) );
	}








/*
	CMediumFilter( int _mediumClass = -1, int _mediumNumber = -1, CAcceptFilter< _RequestResponses > * _accept = 0 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber ),
		m_accept( _accept )
	{}

	~CMediumFilter()
	{
		if ( m_accept )
			delete m_accept;
	}

	int m_mediumClass;
	int m_mediumNumber;

	CAcceptFilter< _RequestResponses >* m_accept;

*/


}

#endif // FILTERS_H
