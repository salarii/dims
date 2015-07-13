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
	std::map< uint160, CTrackerData > trackers;
	CRankingDatabase::getInstance()->loadIdentificationDatabase( trackers );

	std::map< uint160, CTrackerData >::const_iterator iterator = trackers.begin();

	while( iterator != trackers.end() )
	{
		addTracker( iterator->second );

		iterator++;
	}
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
		if ( m_presentTrackers.find( tracker.first ) != m_presentTrackers.end() )
		{
			tracker.second.m_reputation *= PreviousReptationRatio;
			tracker.second.m_reputation += boostForAll;
		}
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

	return 0;
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
CReputationTracker::storeCurrentRanking()
{
	BOOST_FOREACH( RegisteredTrackers::value_type & tracker, m_registeredTrackers )
	{
		CRankingDatabase::getInstance()->writeTrackerData( tracker.second );
	}
}

void
CReputationTracker::loadCurrentRanking()
{
	CRankingDatabase::getInstance()->loadIdentificationDatabase( m_registeredTrackers );
}


void
CReputationTracker::checkValidity( CAllyTrackerData const & _allyTrackerData )
{

}

std::vector< CTrackerData >
CReputationTracker::getTrackers() const
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	std::vector< CTrackerData >trackers;

	BOOST_FOREACH( PAIRTYPE( uint160, CTrackerData ) const & tracker, m_registeredTrackers )
	{
		trackers.push_back( tracker.second );
	}
	return trackers;
}

std::vector< CAllyMonitorData >
CReputationTracker::getAllyMonitors() const
{
	return std::vector< CAllyMonitorData >();
}

std::list< common::CMonitorBaseMedium *>
CReputationTracker::getNodesByClass( common::CMediumKinds::Enum _nodesClass ) const
{
	std::list< common::CMonitorBaseMedium *> mediums;

	uintptr_t nodeIndicator;
	if ( common::CMediumKinds::DimsNodes || common::CMediumKinds::Trackers )
	{

		BOOST_FOREACH( PAIRTYPE( uint160, CTrackerData ) const & trackerData, m_registeredTrackers )
		{
				if ( !getKeyToNode( trackerData.second.m_publicKey, nodeIndicator) )
					assert( !"something wrong" );

				common::CMonitorBaseMedium * medium = findNodeMedium( nodeIndicator );

				if ( !medium )
					assert( !"something wrong" );
				mediums.push_back( medium );
		}
	}
	else if ( common::CMediumKinds::DimsNodes || common::CMediumKinds::Monitors )
	{
		BOOST_FOREACH( PAIRTYPE( uint160, CAllyMonitorData ) const & monitorData, m_monitors )
		{
				if ( !getKeyToNode( monitorData.second.m_key, nodeIndicator) )
					assert( !"something wrong" );

				common::CMonitorBaseMedium * medium = findNodeMedium( nodeIndicator );

				if ( !medium )
					assert( !"something wrong" );
				mediums.push_back( medium );
		}
	}

	return mediums;
}

void
CReputationTracker::setKeyToNode( CPubKey const & _pubKey, uintptr_t _nodeIndicator)
{
	m_pubKeyToNodeIndicator.insert( std::make_pair( _pubKey, _nodeIndicator ) );
}

bool
CReputationTracker::getKeyToNode( CPubKey const & _pubKey, uintptr_t & _nodeIndicator) const
{
	std::map< CPubKey, uintptr_t >::const_iterator iterator = m_pubKeyToNodeIndicator.find( _pubKey );

	if ( iterator != m_pubKeyToNodeIndicator.end() )
		_nodeIndicator = iterator->second;

	return iterator != m_pubKeyToNodeIndicator.end();
}

bool
CReputationTracker::getNodeToKey( uintptr_t _nodeIndicator, CPubKey & _pubKey )const
{
	BOOST_FOREACH( PAIRTYPE(CPubKey, uintptr_t) const & keyToIndicator, m_pubKeyToNodeIndicator )
	{
		if ( keyToIndicator.second == _nodeIndicator )
		{
			_pubKey = keyToIndicator.first;
			return true;
		}
	}

	return false;
}

std::set< common::CValidNodeInfo > const
CReputationTracker::getNodesInfo( common::CRole::Enum _role ) const
{
	std::set< common::CValidNodeInfo > nodesInfo;
	if ( _role == common::CRole::Tracker )
	{
		BOOST_FOREACH( RegisteredTrackers::value_type const & tracker, m_registeredTrackers )
		{
			uintptr_t nodePtr;
			getKeyToNode( tracker.second.m_publicKey, nodePtr );

			CAddress address;
			getAddress( nodePtr, address );

			nodesInfo.insert( common::CValidNodeInfo( tracker.second.m_publicKey, address ) );
		}
	}
	else if ( _role == common::CRole::Monitor )
	{
		BOOST_FOREACH( Monitor::value_type const & monitor, m_monitors )
		{
			uintptr_t nodePtr;
			getKeyToNode( monitor.second.m_publicKey, nodePtr );

			CAddress address;
			getAddress( nodePtr, address );

			nodesInfo.insert( common::CValidNodeInfo( monitor.second.m_publicKey, address ) );
		}
	}
	return nodesInfo;
}

bool
CReputationTracker::checkForTracker( CPubKey const & _pubKey, CTrackerData & _trackerData, CPubKey & _controllingMonitor )const
{

}


void
CReputationTracker::addTracker( CTrackerData const & _trackerData )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_registeredTrackers.insert( std::make_pair( _trackerData.m_publicKey.GetID(), _trackerData ) );
}


}
