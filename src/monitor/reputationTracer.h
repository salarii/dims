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

namespace monitor
{

class CRankingDatabase;

struct CTrackerData
{
	CTrackerData(){}
	CTrackerData( CAddress _address, unsigned int _reputation, CPubKey _publicKey, uint64_t _networkTime, uint64_t _contractTime ): m_address( _address ),m_reputation( _reputation ), m_publicKey( _publicKey ), m_networkTime( _networkTime ), m_contractTime( _contractTime ){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_address);
		READWRITE(m_reputation);
		READWRITE(m_publicKey);
		READWRITE(m_networkTime);
		READWRITE(m_contractTime);
	)

	CAddress m_address;
	unsigned int m_reputation;
	CPubKey m_publicKey;
	uint64_t m_networkTime;
	uint64_t m_contractTime;
};


struct CAllyMonitorData
{
	CAllyMonitorData(){}
	CAllyMonitorData( CAddress _address, CPubKey _publicKey ): m_address( _address ), m_publicKey( _publicKey ){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_address);
		READWRITE(m_publicKey);
	)

	CAddress m_address;
	CPubKey m_publicKey;
};

struct CAllyTrackerData
{
	unsigned int m_reputation;
	uint64_t m_networkTime;
	uint64_t m_previousNetworkTime;
	unsigned int m_countedTime;
};
// for now  don't track other trackers registered in other monitors
class CReputationTracker : public common::CNodesManager< MonitorResponses >
{
public:
	static CReputationTracker * getInstance();

	void addTracker( CTrackerData const & _trackerData );

	void deleteTracker( CPubKey const & _pubKey );

	// both function, not finall form
	std::vector< CTrackerData > getTrackers() const;

	std::vector< CAllyMonitorData > getAllyMonitors() const;

	std::list< common::CMedium< MonitorResponses > *> getNodesByClass( common::CMediumKinds::Enum _nodesClass ) const;

	void setKeyToNode( CPubKey const & _pubKey, uintptr_t _nodeIndicator);

	bool getKeyToNode( CPubKey const & _pubKey, uintptr_t & _nodeIndicator)const;

	bool getNodeToKey( uintptr_t _nodeIndicator, CPubKey & _pubKey )const;

	void setPresentTrackers( std::set< uint160 > const & _presentTrackers )
	{
			boost::lock_guard<boost::mutex> lock( m_lock );
			m_presentTrackers = _presentTrackers;
	}
private:
	CReputationTracker();

	unsigned int calculateReputation( uint64_t _passedTime );

	void checkValidity( CAllyTrackerData const & _allyTrackerData );
	//	boost::lock_guard<boost::mutex> lock( m_lock );
// counting reputation is crucial, it will be  done  differently in separate  action, something I consider  to call "super  action"
	void loop();
private:
	mutable boost::mutex m_lock;

	typedef std::map< uint160, CTrackerData > RegisteredTrackers;

	typedef std::map< uint160, std::vector< CAllyTrackerData > > AllyMonitors;

	typedef std::map< uint160, unsigned int > TransactionsAddmited;

	std::map< uint160, CAllyMonitorData > m_monitors;

	RegisteredTrackers m_registeredTrackers;

	AllyMonitors m_allyMonitorsRankings;

	TransactionsAddmited m_transactionsAddmited;

	std::set< uint160 > m_presentTrackers;

	static CReputationTracker * ms_instance;

	static uint64_t const m_recalculateTime;

	CRankingDatabase * m_rankingDatabase;

	std::map< CPubKey, uintptr_t > m_pubKeyToNodeIndicator;
};

}

#endif


