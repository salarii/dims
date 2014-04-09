// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_LOCAL_RANKING_H
#define TRACKER_LOCAL_RANKING_H

#include <set>
#include <QString>
#include <functional> 

#include "connectionProvider.h"
#include "requestRespond.h"

namespace node
{

class CNetworkClient;

struct CompareBalancedTracker : public std::binary_function<CTrackerStats ,CTrackerStats ,bool>
{
	bool operator() ( CTrackerStats const & _tracker1, CTrackerStats const & _tracker2) const
	{
		return true;
	}
};

struct CompareReputationTracker : public std::binary_function<CTrackerStats ,CTrackerStats ,bool>
{
	bool operator() ( CTrackerStats const & _tracker1, CTrackerStats const & _tracker2) const
	{
		return _tracker1.m_reputation < _tracker2.m_reputation;
	}
};

class CTrackerLocalRanking : public CConnectionProvider
{
public:
	virtual CMedium * provideConnection( RequestKind::Enum const _actionKind );
	// this  will be  rather  complex  stuff  leave  it  for  better  times
	virtual std::list< CMedium *> provideConnection( RequestKind::Enum const _actionKind, unsigned _requestedConnectionNumber );

	float getPrice();

	~CTrackerLocalRanking();

	static CTrackerLocalRanking* getInstance();

	void addTracker( CTrackerStats const & _trackerStats );
private:
	CTrackerLocalRanking();
private:
	static CTrackerLocalRanking * ms_instance;
	// those  sets should be repeatedly rebuild
	std::set< CTrackerStats, CompareBalancedTracker > m_balancedRanking;

	std::set< CTrackerStats, CompareReputationTracker > m_reputationRanking;
};




}

#endif // TRACKER_LOCAL_RANKING_H