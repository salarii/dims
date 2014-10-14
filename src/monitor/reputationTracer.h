// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REPUTATION_TRACER_H
#define REPUTATION_TRACER_H

#include "uint256.h"
#include "key.h"
#include "protocol.h"

#include <boost/thread/mutex.hpp>

namespace monitor
{

struct CTrackerData
{
	CTrackerData( CAddress _address, unsigned int _reputation, CPubKey _publicKey, uint64_t _networkTime ): m_address( _address ),m_reputation( _reputation ), m_publicKey( _publicKey ), m_networkTime( _networkTime ){}

	CAddress m_address;
	unsigned int m_reputation;
	CPubKey m_publicKey;
	uint64_t m_networkTime;
};

struct CAllyTrackerData
{
	unsigned int m_reputation;
	uint64_t m_networkTime;
	uint64_t m_previousNetworkTime;
	unsigned int m_countedTime;
};
// for now  don't track other trackers registered in other monitors
class CReputationTracker
{
public:
	static CReputationTracker * getInstance();

	void addTracker( CTrackerData const & _trackerData );
private:
	CReputationTracker(){};

	unsigned int calculateReputation( uint64_t _passedTime );

	void checkValidity( CAllyTrackerData const & _allyTrackerData );
	//	boost::lock_guard<boost::mutex> lock( m_lock );

	void loop();
private:
	mutable boost::mutex m_lock;

	typedef std::map< uint160, CTrackerData > RegisteredTrackers;

	typedef std::map< uint160, std::vector< CAllyTrackerData > > AllyMonitors;

	typedef std::map< uint160, unsigned int > TransactionsAddmited;

	RegisteredTrackers m_registeredTrackers;

	AllyMonitors m_allyMonitorsRankings;

	TransactionsAddmited m_transactionsAddmited;

	static CReputationTracker * ms_instance;

	static uint64_t const m_recalculateTime;
};

}

#endif


