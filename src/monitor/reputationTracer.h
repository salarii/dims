// Copyright (c) 2014-2015 Dims dev-team
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

	void addAllyTracker( common::CAllyTrackerData const & _trackerData );

	void addAllyMonitor( common::CAllyMonitorData const & _monitorData );

	void deleteTracker( CPubKey const & _pubKey );

	// both function, not finall form
	std::vector< common::CTrackerData > getTrackers() const;

	std::vector< common::CAllyMonitorData > getAllyMonitors() const;

	std::vector< common::CAllyTrackerData > getAllyTrackers() const;

	std::list< common::CMedium *> getNodesByClass( common::CMediumKinds::Enum _nodesClass ) const;

	void setKeyToNode( CPubKey const & _pubKey, uintptr_t _nodeIndicator);

	bool getKeyToNode( CPubKey const & _pubKey, uintptr_t & _nodeIndicator)const;

	bool getNodeToKey( uintptr_t _nodeIndicator, CPubKey & _pubKey )const;

	void setPresentTrackers( std::set< uint160 > const & _presentTrackers )
	{
			boost::lock_guard<boost::mutex> lock( m_lock );
			m_presentTrackers = _presentTrackers;
	}

	void eraseMedium( uintptr_t _nodePtr );

	std::set< common::CValidNodeInfo > const getNodesInfo( common::CRole::Enum _role ) const;

	bool checkForTracker( CPubKey const & _pubKey, common::CTrackerData & _trackerData, CPubKey & _controllingMonitor )const;

	bool eraseExtendInProgress( CPubKey const & _pubKey );
private:
	CReputationTracker();

	void setExtendInProgress( CPubKey const & _pubKey );

	bool isExtendInProgress( CPubKey const & _pubKey );

	unsigned int calculateReputation( uint64_t _passedTime );

	void checkValidity( common::CAllyTrackerData const & _allyTrackerData );

	void storeCurrentRanking();

	void loadCurrentRanking();
// counting reputation is crucial, it will be  done  differently in separate  action, something I consider  to call "super  action"
private:
	mutable boost::mutex m_lock;

	typedef std::map< uint160, common::CTrackerData > RegisteredTrackers;

	typedef std::map< uint160, common::CAllyTrackerData > AllyTrackers;

	typedef std::map< uint160, unsigned int > TransactionsAddmited;

	typedef std::map< uint160, common::CAllyMonitorData > Monitor;

	std::map< uint160, common::CValidNodeInfo > m_candidates;

	std::map< uint160, uint160 > m_trackerToMonitor;

	RegisteredTrackers m_registeredTrackers;

	AllyTrackers m_allyTrackersRankings;

	std::map< uint160, common::CAllyMonitorData > m_allyMonitors;

	TransactionsAddmited m_transactionsAddmited;

	std::set< uint160 > m_presentTrackers;

	static uint64_t const m_recalculateTime;

	std::map< CPubKey, uintptr_t > m_pubKeyToNodeIndicator;

	std::set< CPubKey > m_extendInProgress;
};

}

#endif


