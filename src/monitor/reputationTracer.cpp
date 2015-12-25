// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <math.h>
#include "util.h"

#include <boost/foreach.hpp>

#include "common/actionHandler.h"
#include "common/authenticationProvider.h"

#include "monitor/rankingDatabase.h"
#include "monitor/controller.h"
#include "monitor/admitTrackerAction.h"
#include "monitor/reputationTracer.h"
#include "monitor/activityControllerAction.h"
#include "monitor/pingAction.h"
#include "monitor/updateNetworkDataAction.h"
#include "monitor/reputationControlAction.h"

namespace common
{
std::vector< uint256 > deleteList;
}

namespace monitor
{
double const PreviousReptationRatio = 0.99;// I want  to preserve  a lot

double const TriggerExtendRatio = 0.1;
double const RelativeToMax = 0.3;
unsigned int const OneTransactionGain = 10;
unsigned int const BoostForServicesAlone = 10;


// allow some  deviation for  other monitors

uint64_t const CReputationTracker::m_recalculateTime = 60;// seconds, this  time is  vital how frequent it should be???

CReputationTracker::CReputationTracker()
{
	std::map< uint160, common::CTrackerData > trackers;
	CRankingDatabase::getInstance()->loadIdentificationDatabase( trackers );

	std::map< uint160, common::CTrackerData >::const_iterator iterator = trackers.begin();

	while( iterator != trackers.end() )
	{
		addTracker( iterator->second );
		addNodeToSynch( iterator->second.m_publicKey.GetID() );
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

void
CReputationTracker::calculateReputation()
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	unsigned int  maxTransactionNumber = 0;
	BOOST_FOREACH( TransactionsAddmited::value_type & transactionIndicator, m_transactionsAddmited )
	{
		if ( maxTransactionNumber < transactionIndicator.second )
		{
			maxTransactionNumber = transactionIndicator.second;
		}
	}

	unsigned int boostForAll = ( maxTransactionNumber + BoostForServicesAlone )* OneTransactionGain * RelativeToMax;

	BOOST_FOREACH( PAIRTYPE( uint160 const, common::CTrackerData ) & tracker, m_registeredTrackers )
	{
		tracker.second.m_reputation *= PreviousReptationRatio;
		if ( m_presentNodes.find( tracker.first ) != m_presentNodes.end() )
		{
			tracker.second.m_reputation += boostForAll;
		}
	}

	BOOST_FOREACH( TransactionsAddmited::value_type & transactionIndicator, m_transactionsAddmited )
	{
		std::map< uint160, common::CTrackerData >::iterator iterator = m_registeredTrackers.find( transactionIndicator.first );

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
		std::list< uint160 > toBeRemoved;
		{
			boost::lock_guard<boost::mutex> lock( m_lock );

			BOOST_FOREACH( PAIRTYPE( uint160 const, common::CTrackerData ) & tracker, m_registeredTrackers )
			{
				int64_t timePassed = GetTime() - tracker.second.m_contractTime;
				int64_t timeLeft = tracker.second.m_networkTime - timePassed;

				if ( timeLeft < 0 )
				{
					CRankingDatabase::getInstance()->eraseTrackerData( tracker.second.m_publicKey );
					toBeRemoved.push_back( tracker.second.m_publicKey.GetID() );
					m_allowSynchronization.erase( tracker.second.m_publicKey.GetID() );
				}
				else if ( timeLeft < CController::getInstance()->getPeriod() * TriggerExtendRatio )
				{
					if ( !isExtendInProgress( tracker.second.m_publicKey ) )
					{
						setExtendInProgress( tracker.second.m_publicKey );
						common::CActionHandler::getInstance()->executeAction( new CAdmitTrackerAction(tracker.second.m_publicKey) );
					}
				}
				else
				{
					if ( isExtendInProgress( tracker.second.m_publicKey ) )
						m_extendInProgress.erase( tracker.second.m_publicKey );
				}
			}

			BOOST_FOREACH( uint160 const & id, toBeRemoved )
			{
				m_registeredTrackers.erase( id );
			}

		}

		// outside of  lock  scope
		common::CRankingFullInfo rankingFullInfo(
					getAllyTrackers()
					, getAllyMonitors()
					, getTrackers()
					, getSynchronizedTrackers()
					, getMeasureReputationTime()
					, CReputationControlAction::getInstance()->getActionKey() );

		if ( !toBeRemoved.empty() )
		{
			common::CActionHandler::getInstance()->executeAction( new CUpdateNetworkDataAction( rankingFullInfo, common::CMediumKinds::DimsNodes ) );
		}

		MilliSleep( 1000 );
	}
}

void
CReputationTracker::storeCurrentRanking()
{
	BOOST_FOREACH( PAIRTYPE( uint160 const, common::CTrackerData ) & tracker, m_registeredTrackers )
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
CReputationTracker::setNodeInfo( common::CValidNodeInfo const & _validNodeInfo, common::CRole::Enum _role )
{
	switch( _role )
	{
	case common::CRole::Seed:
		break;
	case common::CRole::Tracker:
		m_knownTrackers.insert( _validNodeInfo );
		break;
	case common::CRole::Monitor:
		m_knownMonitors.insert( _validNodeInfo );
		break;
	default:
		break;
	}
}

void
CReputationTracker::clearTransactions()
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_transactionsAddmited.clear();
}

void
CReputationTracker::recalculateReputation()
{
	m_measureReputationTime += m_recalculateTime;
	calculateReputation();
}

void
CReputationTracker::checkValidity( common::CAllyTrackerData const & _allyTrackerData )
{
}

std::set< common::CTrackerData >
CReputationTracker::getTrackers() const
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	std::set< common::CTrackerData >trackers;

	BOOST_FOREACH( PAIRTYPE( uint160, common::CTrackerData ) const & tracker, m_registeredTrackers )
	{
		trackers.insert( tracker.second );
	}
	return trackers;
}

std::set< common::CAllyMonitorData >
CReputationTracker::getAllyMonitors() const
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	std::set< common::CAllyMonitorData > monitors;

	BOOST_FOREACH( PAIRTYPE( uint160, common::CAllyMonitorData ) const & monitor, m_allyMonitors )
	{
		monitors.insert( monitor.second );
	}

	return monitors;
}

std::set< common::CAllyTrackerData >
CReputationTracker::getAllyTrackers() const
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	std::set< common::CAllyTrackerData > trackers;

	BOOST_FOREACH( PAIRTYPE( uint160, common::CAllyTrackerData ) const & tracker, m_allyTrackersRankings )
	{
		trackers.insert( tracker.second );
	}

	return trackers;
}

std::list< common::CMedium *>
CReputationTracker::provideConnection( common::CMediumFilter const & _mediumFilter )
{
	std::list< common::CMedium*> mediums = common::CNodesManager::provideConnection( _mediumFilter );

	if ( !mediums.empty() )
		return mediums;

	return _mediumFilter.getMediums( this );
}

std::list< common::CMedium *>
CReputationTracker::getNodesByClass( common::CMediumKinds::Enum _nodesClass ) const
{
	std::list< common::CMedium *> mediums;

	uintptr_t nodeIndicator;

	if ( !CController::getInstance()->isAdmitted() )
	{
		if ( _nodesClass == common::CMediumKinds::DimsNodes
			 || _nodesClass == common::CMediumKinds::Monitors
			 || _nodesClass == common::CMediumKinds::Trackers
			 )
		{
			if ( _nodesClass != common::CMediumKinds::Trackers )
			{
				BOOST_FOREACH( common::CValidNodeInfo const & validNode, m_knownMonitors )
				{
					// in case  of  fail  do  something ??
					if ( getKeyToNode( validNode.m_publicKey.GetID(), nodeIndicator) )
					{
						common::CMedium * medium = findNodeMedium( nodeIndicator );
						if ( medium )
							mediums.push_back( medium );
					}
				}
			}

			if ( _nodesClass != common::CMediumKinds::Monitors )
			{
				BOOST_FOREACH( common::CValidNodeInfo const & validNode, m_knownTrackers )
				{
					if ( getKeyToNode( validNode.m_publicKey.GetID(), nodeIndicator) )
					{
						common::CMedium * medium = findNodeMedium( nodeIndicator );
						if ( medium )
							mediums.push_back( medium );
					}
				}
			}
			return mediums;
		}
	}
	else
	{
		if ( _nodesClass == common::CMediumKinds::DimsNodes
			 || _nodesClass == common::CMediumKinds::Monitors
			 || _nodesClass == common::CMediumKinds::Trackers
			 )
		{
			if ( _nodesClass != common::CMediumKinds::Monitors )
			{

				BOOST_FOREACH( PAIRTYPE( uint160, common::CTrackerData ) const & trackerData, m_registeredTrackers )
				{
					if ( m_presentNodes.find( trackerData.first ) != m_presentNodes.end() && m_synchronizedTrackers.find( trackerData.first ) != m_synchronizedTrackers.end() )
					{
						if ( !getKeyToNode( trackerData.second.m_publicKey.GetID(), nodeIndicator) )
							assert( !"something wrong" );

						common::CMedium * medium = findNodeMedium( nodeIndicator );

						if ( !medium )
							assert( !"something wrong" );
						mediums.push_back( medium );
					}
				}

				BOOST_FOREACH( PAIRTYPE( uint160, common::CAllyTrackerData ) const & trackerData, m_allyTrackersRankings )
				{
					if ( m_presentNodes.find( trackerData.first ) != m_presentNodes.end() && m_synchronizedTrackers.find( trackerData.first ) != m_synchronizedTrackers.end() )
					{
						if ( !getKeyToNode( trackerData.second.m_publicKey.GetID(), nodeIndicator) )
							assert( !"something wrong" );

						common::CMedium * medium = findNodeMedium( nodeIndicator );

						if ( !medium )
							assert( !"something wrong" );
						mediums.push_back( medium );
					}
				}
			}

			if ( _nodesClass != common::CMediumKinds::Trackers )
			{
				BOOST_FOREACH( PAIRTYPE( uint160, common::CAllyMonitorData ) const & monitorData, m_allyMonitors )
				{
					if ( m_presentNodes.find( monitorData.first ) != m_presentNodes.end() )
					{
						if ( !getKeyToNode( monitorData.second.m_publicKey.GetID(), nodeIndicator) )
							assert( !"something wrong" );

						common::CMedium * medium = findNodeMedium( nodeIndicator );

						if ( !medium )
							assert( !"something wrong" );
						mediums.push_back( medium );
					}
				}
			}
		}
	}
	return mediums;
}

void
CReputationTracker::setKeyToNode( CPubKey const & _pubKey, uintptr_t _nodeIndicator)
{
	m_pubKeyToNodeIndicator.erase(_pubKey);
	m_pubKeyToNodeIndicator.insert( std::make_pair( _pubKey, _nodeIndicator ) );
}

bool
CReputationTracker::getKeyToNode( uint160 const & _pubKeyId, uintptr_t & _nodeIndicator) const
{
	std::map< CPubKey, uintptr_t >::const_iterator iterator =
			m_pubKeyToNodeIndicator.begin();

	while( iterator != m_pubKeyToNodeIndicator.end() )
	{
		if ( iterator->first.GetID() == _pubKeyId )
			break;
		iterator++;
	}


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

std::set< uint160 >
CReputationTracker::getPresentAndSynchronizedTrackers() const
{
	std::set< common::CTrackerData > trackers = getTrackers();

	std::set< uint160 > presentTrackers;
	BOOST_FOREACH( common::CTrackerData const & trackerData, trackers )
	{
		if ( isPresentNode( trackerData.m_publicKey.GetID() ) && isTrackerSynchronized( trackerData.m_publicKey.GetID() ) )
			presentTrackers.insert( trackerData.m_publicKey.GetID() );
	}

	std::set< common::CAllyTrackerData > allyTrackers = getAllyTrackers();

	BOOST_FOREACH( common::CAllyTrackerData const & allyTracker, allyTrackers )
	{
		if ( isPresentNode( allyTracker.m_publicKey.GetID() ) && isTrackerSynchronized( allyTracker.m_publicKey.GetID() ) )
			presentTrackers.insert( allyTracker.m_publicKey.GetID() );
	}

	return presentTrackers;
}

void
CReputationTracker::eraseMedium( uintptr_t _nodePtr )
{
	CPubKey key;

	boost::lock_guard<boost::mutex> lock( m_lock );

	if ( !getNodeToKey( _nodePtr, key ) )
		return;

	CKeyID keyId = key.GetID();

	m_candidates.erase( keyId );

	m_transactionsAddmited.erase( keyId );

	m_presentNodes.erase( keyId );

	m_pubKeyToNodeIndicator.erase( key );

	CAddress address;
	if ( getAddress( _nodePtr, address ) )
	{
		erasePubKey( address );

		common::CNodesManager::eraseMedium( _nodePtr );

		common::CActionHandler::getInstance()->executeAction( new CActivityControllerAction( key, address, CActivitySatatus::Inactive ) );
	}
}

void
CReputationTracker::evaluateNode( common::CSelfNode * _selfNode )
{
	CPubKey pubKey;
	if ( getPublicKey( _selfNode->addr, pubKey ) )
	{
		common::CActionHandler::getInstance()->executeAction( new CPingAction( _selfNode ) );
	}
	else
	{
		_selfNode->fDisconnect = true;  // for seed  and unknown
	}
}
std::set< common::CValidNodeInfo > const
CReputationTracker::getNodesInfo( common::CRole::Enum _role ) const
{
	std::set< common::CValidNodeInfo > nodesInfo;
	if ( _role == common::CRole::Tracker )
	{
		BOOST_FOREACH( PAIRTYPE( uint160 const, common::CTrackerData ) const & tracker, m_registeredTrackers )
		{
			uintptr_t nodePtr;
			getKeyToNode( tracker.second.m_publicKey.GetID(), nodePtr );

			CAddress address;
			getAddress( nodePtr, address );

			nodesInfo.insert( common::CValidNodeInfo( tracker.second.m_publicKey, address ) );
		}
	}
	else if ( _role == common::CRole::Monitor )
	{
		BOOST_FOREACH( PAIRTYPE( uint160, common::CAllyMonitorData ) const & monitor, m_allyMonitors )
		{
			uintptr_t nodePtr;
			getKeyToNode( monitor.second.m_publicKey.GetID(), nodePtr );

			CAddress address;
			getAddress( nodePtr, address );

			nodesInfo.insert( common::CValidNodeInfo( monitor.second.m_publicKey, address ) );
		}
	}
	return nodesInfo;
}

bool
CReputationTracker::checkForTracker( uint160 const & _pubKeyId, common::CTrackerData & _trackerData, CPubKey & _controllingMonitor )const
{
	std::map< uint160, common::CTrackerData >::const_iterator iterator = m_registeredTrackers.find( _pubKeyId );

	if ( iterator != m_registeredTrackers.end() )
	{
		_trackerData = iterator->second;
		_controllingMonitor = common::CAuthenticationProvider::getInstance()->getMyKey();

		return true;
	}

	std::map< uint160, common::CAllyTrackerData >::const_iterator allyIterator = m_allyTrackersRankings.find( _pubKeyId );

	if ( allyIterator == m_allyTrackersRankings.end() )
		return  false;

	_trackerData = dynamic_cast<common::CTrackerData const&>( allyIterator->second );

	_controllingMonitor = allyIterator->second.m_allyMonitorKey;

	return true;
}

void
CReputationTracker::setExtendInProgress( CPubKey const & _pubKey )
{
		m_extendInProgress.insert( _pubKey );
}

bool
CReputationTracker::isExtendInProgress( CPubKey const & _pubKey )
{
	return m_extendInProgress.find(_pubKey) != m_extendInProgress.end();
}

bool
CReputationTracker::eraseExtendInProgress( CPubKey const & _pubKey )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_extendInProgress.erase(_pubKey);

	return true;
}

bool
CReputationTracker::isAddmitedMonitor( uint160 const & _pubKeyId )
{
	return m_allyMonitors.find( _pubKeyId ) != m_allyMonitors.end();
}

bool
CReputationTracker::isRegisteredTracker( uint160 const & _pubKeyId )
{
	if ( m_registeredTrackers.find( _pubKeyId ) != m_registeredTrackers.end() )
		return true;

	return m_allyTrackersRankings.find( _pubKeyId ) != m_allyTrackersRankings.end();

}

void
CReputationTracker::addTracker( common::CTrackerData const & _trackerData )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_registeredTrackers.erase( _trackerData.m_publicKey.GetID() );
	m_registeredTrackers.insert( std::make_pair( _trackerData.m_publicKey.GetID(), _trackerData ) );
}

bool
CReputationTracker::getTracker( uint160 const & _pubKeyId, common::CTrackerData & _trackerData ) const
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	std::map< uint160, common::CTrackerData >::const_iterator iterator =
			m_registeredTrackers.find( _pubKeyId );

	if ( iterator != m_registeredTrackers.end() )
	{
		_trackerData = (*iterator).second;
		return true;
	}

	return false;
}

void
CReputationTracker::removeTracker( uint160 const & _pubKeyId )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_registeredTrackers.find( _pubKeyId );
}

void
CReputationTracker::addAllyTracker( common::CAllyTrackerData const & _trackerData )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	m_allyTrackersRankings.erase( _trackerData.m_publicKey.GetID() );
	m_allyTrackersRankings.insert( make_pair( _trackerData.m_publicKey.GetID(), _trackerData ) );

	m_trackerToMonitor.erase( _trackerData.m_publicKey.GetID() );
	m_trackerToMonitor.insert( std::make_pair( _trackerData.m_publicKey.GetID(), _trackerData.m_allyMonitorKey.GetID() ) );
}

void
CReputationTracker::removeAllyTracker( uint160 const & _pubKeyId )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_allyTrackersRankings.erase( _pubKeyId );
	m_trackerToMonitor.erase( _pubKeyId );
}

void
CReputationTracker::addAllyMonitor( common::CAllyMonitorData const & _monitorData )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	if ( !(common::CAuthenticationProvider::getInstance()->getMyKey() == _monitorData.m_publicKey) )
	{
		m_allyMonitors.insert( std::make_pair( _monitorData.m_publicKey.GetID(), _monitorData ) );
	}
}

void
CReputationTracker::removeAllyMonitor( uint160 const & _pubKeyId )
{
	boost::lock_guard<boost::mutex> lock( m_lock );

	m_allyMonitors.erase( _pubKeyId );
}

void
CReputationTracker::clearRankingData()
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_trackerToMonitor.clear();
	m_registeredTrackers.clear();
	m_allyTrackersRankings.clear();
	m_allyMonitors.clear();
	m_presentNodes.clear();
	m_extendInProgress.clear();

	CRankingDatabase::getInstance()->resetDb();
}

bool
CReputationTracker::isSynchronizationAllowed( uint160 const & _pubKeyId )const
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	return m_allowSynchronization.find( _pubKeyId ) != m_allowSynchronization.end();
}

void
CReputationTracker::removeNodeFromSynch( uint160 const & _pubKeyId )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_allowSynchronization.erase( _pubKeyId );
}

void
CReputationTracker::addNodeToSynch( uint160 const & _pubKeyId )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	m_allowSynchronization.insert( _pubKeyId );
}

bool
CReputationTracker::getAddresFromKey( uint160 const & _pubKeyId, CAddress & _address )const
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	uintptr_t nodeIndicator;
	if ( !getKeyToNode( _pubKeyId, nodeIndicator ) )
		return false;

	return getAddress( nodeIndicator, _address );
}

void
CReputationTracker::updateRankingInfo( CPubKey const & _pubKey, common::CRankingFullInfo const & _rankingFullInfo )
{
	boost::lock_guard<boost::mutex> lock( m_lock );
	// sanity, TODO: later react if wrong

	if ( m_allyMonitors.find( _pubKey.GetID() ) == m_allyMonitors.end() )
		return;

	BOOST_FOREACH( common::CAllyTrackerData const & allyTrackerData, _rankingFullInfo.m_allyTrackers )
	{
		if (
					m_registeredTrackers.find( allyTrackerData.m_publicKey.GetID() ) == m_registeredTrackers.end()
				&& m_allyTrackersRankings.find( allyTrackerData.m_publicKey.GetID() ) == m_allyTrackersRankings.end()
					)
		{
			//assert( !"react  to  this" );
			//  problem  here  but  work on it  another  day
		}
	}

	BOOST_FOREACH( common::CAllyMonitorData const & allyMonitorData, _rankingFullInfo.m_allyMonitors )
	{
		if ( m_knownMonitors.find( common::CValidNodeInfo( allyMonitorData.m_publicKey, allyMonitorData.m_address ) ) == m_knownMonitors.end() )
			;//create  connect  action ??

		if ( m_allyMonitors.find( allyMonitorData.m_publicKey.GetID() ) == m_allyMonitors.end() )
		{
			if ( !(common::CAuthenticationProvider::getInstance()->getMyKey() == allyMonitorData.m_publicKey) )
			{
				m_allyMonitors.insert( std::make_pair( allyMonitorData.m_publicKey.GetID(), allyMonitorData ) );
			}
		}
	}

	//add new tracker data, if any

	BOOST_FOREACH( common::CTrackerData const & trackerData, _rankingFullInfo.m_trackers )
	{
		if ( m_allyTrackersRankings.find( trackerData.m_publicKey.GetID() ) == m_allyTrackersRankings.end() )
		{
			if ( m_knownTrackers.find( common::CValidNodeInfo( trackerData.m_publicKey, trackerData.m_address ) ) == m_knownTrackers.end() )
				;//create  connect  action ??

			m_allyTrackersRankings.insert( make_pair( trackerData.m_publicKey.GetID(), common::CAllyTrackerData( trackerData, _pubKey ) ) );
			m_trackerToMonitor.insert( std::make_pair( trackerData.m_publicKey.GetID(), _pubKey.GetID() ) );
		}
	}

	BOOST_FOREACH( uint160 const & keyId, _rankingFullInfo.m_synchronizedTrackers )
	{
		m_synchronizedTrackers.insert( keyId );
	}

}

}
