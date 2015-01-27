// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "reputationTracer.h"

#include <math.h>
#include "util.h"

#include <boost/foreach.hpp>

#include "monitor/rankingDatabase.h"

namespace monitor
{
double const PreviousReptationRatio = 0.95;// I want  to preserve  a lot
double const RelativeToMax = 0.3;
unsigned int const OneTransactionGain = 10;

// allow some  deviation for  other monitors

CReputationTracker * CReputationTracker::ms_instance = NULL;

uint64_t const CReputationTracker::m_recalculateTime = 10000;// this  time is  vital how frequent it should be???

CReputationTracker::CReputationTracker()
{
	m_rankingDatabase = new CRankingDatabase( "rankingData", "rc+" );
}

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


	unsigned int  maxTransactionNumber = 0;
	BOOST_FOREACH( TransactionsAddmited::value_type & transactionIndicator, m_transactionsAddmited )
	{
		if ( maxTransactionNumber < transactionIndicator.second )
		{
			maxTransactionNumber = transactionIndicator.second;
		}
	}

	unsigned int boostForAll = ( maxTransactionNumber + 1 )* OneTransactionGain * RelativeToMax;

	BOOST_FOREACH( RegisteredTrackers::value_type & tracker, m_registeredTrackers )
	{
		tracker.second.m_reputation *= PreviousReptationRatio;
		tracker.second.m_reputation += boostForAll;
	}

	BOOST_FOREACH( TransactionsAddmited::value_type & transactionIndicator, m_transactionsAddmited )
	{
		RegisteredTrackers::iterator iterator = m_registeredTrackers.find( transactionIndicator.first );

		if ( m_registeredTrackers.end() != iterator )
		{
			iterator->second.m_reputation += transactionIndicator.second * OneTransactionGain;
		}
	}

	m_transactionsAddmited.clear();
}

void
CReputationTracker::loop()
{
	while( 1 )
	{
		boost::lock_guard<boost::mutex> lock( m_lock );
		BOOST_FOREACH( RegisteredTrackers::value_type & tracker, m_registeredTrackers )
		{
			if ( tracker.second.m_networkTime <= m_recalculateTime )
			{
				// tracker need to pay again
				// create new  pay  action
			}
			else
			{
				tracker.second.m_reputation = calculateReputation( tracker.second.m_networkTime );
			}
		//	tracker.second.m_networkTime += m_recalculateTime;
		}
/*
		BOOST_FOREACH( AllyMonitors::value_type & monitor, m_allyMonitorsRankings )
		{
			BOOST_FOREACH( CAllyTrackerData & tracker, monitor.second )
			{
				checkValidity( tracker );
			}
		}
*/
		MilliSleep( m_recalculateTime );
	}
}

void
CReputationTracker::checkValidity( CAllyTrackerData const & _allyTrackerData )
{

}

void
CReputationTracker::addTracker( CTrackerData const & _trackerData )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_registeredTrackers.insert( std::make_pair( _trackerData.m_publicKey.GetID(), _trackerData ) );
}


}
