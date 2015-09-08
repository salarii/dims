// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "reputationTracer.h"

#include <math.h>
#include "util.h"

#include <boost/foreach.hpp>

#include "common/actionHandler.h"

#include "monitor/rankingDatabase.h"
#include "monitor/monitorController.h"
#include "monitor/admitTrackerAction.h"

namespace common
{
std::vector< uint256 > deleteList;

template<> CNodesManager< common::CMonitorTypes > * common::CNodesManager< common::CMonitorTypes >::ms_instance = 0;
}

namespace monitor
{
double const PreviousReptationRatio = 0.95;// I want  to preserve  a lot

double const TriggerExtendRatio = 0.1;
double const RelativeToMax = 0.3;
unsigned int const OneTransactionGain = 10;



// allow some  deviation for  other monitors

uint64_t const CReputationTracker::m_recalculateTime = 20000;// this  time is  vital how frequent it should be???

CReputationTracker::CReputationTracker()
{
	std::map< uint160, common::CTrackerData > trackers;
	CRankingDatabase::getInstance()->loadIdentificationDatabase( trackers );

	std::map< uint160, common::CTrackerData >::const_iterator iterator = trackers.begin();

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
	return dynamic_cast<CReputationTracker *>( ms_instance );
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
		{
			boost::lock_guard<boost::mutex> lock( m_lock );

			std::list< uint160 > toBeRemoved;
			BOOST_FOREACH( RegisteredTrackers::value_type & tracker, m_registeredTrackers )
			{
				int64_t timePassed = GetTime() - tracker.second.m_contractTime;
				int64_t timeLeft = tracker.second.m_networkTime - timePassed;

				//	timeLeft < 0//  unregister
				if ( timeLeft < 0 )
				{
					toBeRemoved.push_back( tracker.first );
					CRankingDatabase::getInstance()->eraseTrackerData( tracker.second.m_publicKey );
				}
				else if ( timeLeft < CMonitorController::getInstance()->getPeriod() * TriggerExtendRatio )
				{
					uintptr_t nodeIndicator;
					getKeyToNode( tracker.second.m_publicKey, nodeIndicator);
					if ( isExtendInProgress( tracker.second.m_publicKey ) )
					{
						setExtendInProgress( tracker.second.m_publicKey );
						common::CActionHandler< common::CMonitorTypes >::getInstance()->executeAction( new CAdmitTrackerAction(nodeIndicator) );
					}
				}

				{
					tracker.second.m_reputation = calculateReputation( tracker.second.m_networkTime );
				}
				//	tracker.second.m_networkTime += m_recalculateTime;
			}

			BOOST_FOREACH( uint160 const & id, toBeRemoved )
			{
				m_registeredTrackers.erase( id );
			}
		}

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
CReputationTracker::checkValidity( common::CAllyTrackerData const & _allyTrackerData )
{
}

std::vector< common::CTrackerData >
CReputationTracker::getTrackers() const
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	std::vector< common::CTrackerData >trackers;

	BOOST_FOREACH( PAIRTYPE( uint160, common::CTrackerData ) const & tracker, m_registeredTrackers )
	{
		trackers.push_back( tracker.second );
	}
	return trackers;
}

std::vector< common::CAllyMonitorData >
CReputationTracker::getAllyMonitors() const
{
	return std::vector< common::CAllyMonitorData >();
}

std::vector< common::CAllyTrackerData >
CReputationTracker::getAllyTrackers() const
{
	return std::vector< common::CAllyTrackerData >();
}

std::list< common::CMonitorBaseMedium *>
CReputationTracker::getNodesByClass( common::CMediumKinds::Enum _nodesClass ) const
{
	std::list< common::CMonitorBaseMedium *> mediums;

	uintptr_t nodeIndicator;
	if ( common::CMediumKinds::DimsNodes || common::CMediumKinds::Trackers )
	{

		BOOST_FOREACH( PAIRTYPE( uint160, common::CTrackerData ) const & trackerData, m_registeredTrackers )
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
		BOOST_FOREACH( PAIRTYPE( uint160, common::CAllyMonitorData ) const & monitorData, m_allyMonitors )
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

void
CReputationTracker::eraseMedium( uintptr_t _nodePtr )
{
	//reconsider  what  needs  to  be cleared

	boost::lock_guard<boost::mutex> lock( m_lock );

	common::CNodesManager< common::CMonitorTypes >::eraseMedium( _nodePtr );

	CPubKey pubKey;

	getNodeToKey( _nodePtr, pubKey );

	uint160 keyId = pubKey.GetID();

	m_candidates.erase( keyId );

	m_allyMonitors.erase( keyId );

	m_trackerToMonitor.erase( keyId );

	m_transactionsAddmited.erase( keyId );

	m_presentTrackers.erase( keyId );

	m_pubKeyToNodeIndicator.erase( pubKey );
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
		BOOST_FOREACH( Monitor::value_type const & monitor, m_allyMonitors )
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
CReputationTracker::checkForTracker( CPubKey const & _pubKey, common::CTrackerData & _trackerData, CPubKey & _controllingMonitor )const
{
	RegisteredTrackers::const_iterator iterator = m_registeredTrackers.find( _pubKey.GetID() );

	if ( iterator != m_registeredTrackers.end() )
	{
		_trackerData = iterator->second;
		_controllingMonitor = common::CAuthenticationProvider::getInstance()->getMyKey();

		return true;
	}

	AllyTrackers::const_iterator allyIterator = m_allyTrackersRankings.find( _pubKey.GetID() );

	if ( allyIterator == m_allyTrackersRankings.end() )
		return  false;

	_trackerData = dynamic_cast<common::CTrackerData const&>( allyIterator->second );

	_controllingMonitor = allyIterator->second.m_allyMonitorKey;

	return true;
}

void
CReputationTracker::setExtendInProgress( CPubKey const & _pubKey )
{
		boost::lock_guard<boost::mutex> lock( m_lock );
		m_extendInProgress.insert( _pubKey );
}

bool
CReputationTracker::isExtendInProgress( CPubKey const & _pubKey )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	return m_extendInProgress.find(_pubKey) != m_extendInProgress.end();
}

bool
CReputationTracker::eraseExtendInProgress( CPubKey const & _pubKey )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_extendInProgress.erase(_pubKey);//m_extendInProgress.find(_pubKey)

	return true;
}

void
CReputationTracker::addTracker( common::CTrackerData const & _trackerData )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_registeredTrackers.insert( std::make_pair( _trackerData.m_publicKey.GetID(), _trackerData ) );
}

void
CReputationTracker::addAllyTracker( common::CAllyTrackerData const & _trackerData )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	//m_allyTrackersRankings.push_back( _trackerData );
}

void
CReputationTracker::addAllyMonitor( common::CAllyMonitorData const & _monitorData )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_allyMonitors.insert( std::make_pair( _monitorData.m_key.GetID(), _monitorData ) );
}

}
