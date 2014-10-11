// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "reputationTracer.h"

#include <math.h>
#include "util.h"

#include <boost/foreach.hpp>

namespace monitor
{

CReputationTracker * CReputationTracker::ms_instance = NULL;

uint64_t const CReputationTracker::m_recalculateTime = 10000;

CReputationTracker*
CReputationTracker::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CReputationTracker();
	};
	return ms_instance;
}

unsigned int
CReputationTracker::calculateReputation( uint64_t _passedTime )
{
	;
}

void
CReputationTracker::loop()
{
	while( 1 )
	{
		BOOST_FOREACH( RegisteredTrackers::value_type & tracker, m_registeredTrackers )
		{
		//	tracker.second.m_networkTime += m_recalculateTime;

		//	tracker.second.m_reputation = calculateReputation( tracker.second.m_networkTime );

		}
// make  estimation
		BOOST_FOREACH( AllyMonitors::value_type & monitor, m_allyMonitorsRankings )
		{
			BOOST_FOREACH( CAllyTrackerData & tracker, monitor.second )
			{
				checkValidity( tracker );
			}
		}

		MilliSleep( m_recalculateTime );
	}
}

void
CReputationTracker::checkValidity( CAllyTrackerData const & _allyTrackerData )
{

}

void
CReputationTracker::addTracker( uint160 const _trackerKeyId )
{
	m_transactionsAddmited.insert( std::make_pair( _trackerKeyId, 0 ) );
//	m_registeredTrackers.insert( std::make_pair( _trackerKeyId, CTrackerData() ) );
}


}
