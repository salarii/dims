// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REPUTATION_TRACER_H
#define REPUTATION_TRACER_H

#include "uint256.h"
#include "key.h"
#include "protocol.h"

#include <boost/thread/mutex.hpp>

#include "common/nodesManager.h"
#include "common/struct.h"

namespace monitor
{

// for now  don't track other trackers registered in other monitors
class CReputationTracker : public common::CNodesManager
{
public:
	static CReputationTracker * getInstance();

	void loop();

	void addTracker( common::CTrackerData const & _trackerData );

	bool getTracker( uint160 const & _pubKeyId, common::CTrackerData & _trackerData ) const;

	bool isRegisteredTracker( uint160 const & _pubKeyId );

	void addAllyTracker( common::CAllyTrackerData const & _trackerData );

	void addAllyMonitor( common::CAllyMonitorData const & _monitorData );

	// both function, not final form
	std::set< common::CTrackerData > getTrackers() const;

	std::set< common::CAllyMonitorData > getAllyMonitors() const;

	std::set< common::CAllyTrackerData > getAllyTrackers() const;

	std::list< common::CMedium *> getNodesByClass( common::CMediumKinds::Enum _nodesClass ) const;

	std::list< common::CMedium *> provideConnection( common::CMediumFilter const & _mediumFilter );

	void setKeyToNode( CPubKey const & _pubKey, uintptr_t _nodeIndicator);

	bool getKeyToNode( uint160 const & _pubKeyId, uintptr_t & _nodeIndicator)const;

	bool getNodeToKey( uintptr_t _nodeIndicator, CPubKey & _pubKey )const;

	void setPresentNode( uint160 const & _nodeId )
	{
			boost::lock_guard<boost::mutex> lock( m_lock );
			m_presentNodes.insert( _nodeId );
	}

	void erasePresentNode( uint160 const & _nodeId )
	{
		boost::lock_guard<boost::mutex> lock( m_lock );
		m_presentNodes.erase( _nodeId );
	}

	bool isPresentNode( uint160 const & _nodeId )const
	{
		boost::lock_guard<boost::mutex> lock( m_lock );
		return m_presentNodes.find( _nodeId ) != m_presentNodes.end();
	}

	std::set< uint160 > getPresentTrackers() const;

	void eraseMedium( uintptr_t _nodePtr );

	void evaluateNode( common::CSelfNode * _selfNode );

	std::set< common::CValidNodeInfo > const getNodesInfo( common::CRole::Enum _role ) const;

	bool checkForTracker( uint160 const & _pubKeyId, common::CTrackerData & _trackerData, CPubKey & _controllingMonitor )const;

	bool eraseExtendInProgress( CPubKey const & _pubKey );

	bool isAddmitedMonitor( uint160 const & _pubKeyId );

	void setNodeInfo( common::CValidNodeInfo const & _validNodeInfo, common::CRole::Enum _role );

	static uint64_t getRecalculateTime(){ return m_recalculateTime; }

	void setMeasureReputationTime( uint64_t _measureReputationTime ){ m_measureReputationTime = _measureReputationTime; }

	uint64_t getMeasureReputationTime()const{ return m_measureReputationTime; }

	void clearTransactions();

	void recalculateReputation();

	void clearRankingData();
// ugly
	bool isSynchronizationAllowed( uint160 const & _pubKeyId )const;

	void removeNodeFromSynch( uint160 const & _pubKeyId );

	void addNodeToSynch( uint160 const & _pubKeyId );

	void updateRankingInfo( CPubKey const & _pubKey, common::CRankingFullInfo const & _rankingFullInfo );

	bool getAddresFromKey( uint160 const & _pubKeyId, CAddress & _address )const;
private:

	CReputationTracker();

	void setExtendInProgress( CPubKey const & _pubKey );

	bool isExtendInProgress( CPubKey const & _pubKey );

	void calculateReputation();

	void checkValidity( common::CAllyTrackerData const & _allyTrackerData );

	void storeCurrentRanking();

	void loadCurrentRanking();
private:
	mutable boost::mutex m_lock;

	typedef std::map< uint160, unsigned int > TransactionsAddmited;

	std::set< common::CValidNodeInfo > m_knownTrackers;

	std::set< common::CValidNodeInfo > m_knownMonitors;

	std::map< uint160, common::CValidNodeInfo > m_candidates;

	std::map< uint160, uint160 > m_trackerToMonitor;

	std::map< uint160, common::CTrackerData > m_registeredTrackers;

	std::map< uint160, common::CAllyTrackerData > m_allyTrackersRankings;

	std::map< uint160, common::CAllyMonitorData > m_allyMonitors;

	TransactionsAddmited m_transactionsAddmited;

	std::set< uint160 > m_presentNodes;

	static uint64_t const m_recalculateTime;

	uint64_t m_measureReputationTime;

	std::map< CPubKey, uintptr_t > m_pubKeyToNodeIndicator;

	std::set< CPubKey > m_extendInProgress;

	std::set< uint160 > m_allowSynchronization;
};

}

#endif


