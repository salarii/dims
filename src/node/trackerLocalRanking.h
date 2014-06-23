// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_LOCAL_RANKING_H
#define TRACKER_LOCAL_RANKING_H

#include <set>
#include <map>
#include <QString>
#include <functional> 

#include "common/connectionProvider.h"
#include "common/requestResponse.h"
#include "configureNodeActionHadler.h"

namespace node
{

class CNetworkClient;

struct CompareBalancedTracker : public std::binary_function< common::CTrackerStats , common::CTrackerStats ,bool>
{
	bool operator() ( common::CTrackerStats const & _tracker1, common::CTrackerStats const & _tracker2) const
	{
		return true;
	}
};

struct CompareReputationTracker : public std::binary_function< common::CTrackerStats ,common::CTrackerStats ,bool>
{
	bool operator() ( common::CTrackerStats const & _tracker1, common::CTrackerStats const & _tracker2) const
	{
		return _tracker1.m_reputation < _tracker2.m_reputation;
	}
};

class CTrackerLocalRanking : public common::CConnectionProvider< NodeResponses >
{
public:
	virtual std::list< common::CMedium< NodeResponses > *> provideConnection( int const _actionKind, unsigned _requestedConnectionNumber );

	float getPrice();

	~CTrackerLocalRanking();

	static CTrackerLocalRanking* getInstance();

	void addTracker( common::CTrackerStats const & _trackerStats );

private:
	CTrackerLocalRanking();

	common::CMedium< NodeResponses > * getNetworkConnection( common::CTrackerStats const & _trackerStats );
private:
	static CTrackerLocalRanking * ms_instance;
	// those  sets should be repeatedly rebuild
	std::set< common::CTrackerStats, CompareBalancedTracker > m_balancedRanking;

	std::set< common::CTrackerStats, CompareReputationTracker > m_reputationRanking;

	std::map< std::string, common::CMedium< NodeResponses > * > m_createdMediums;
};




}

#endif // TRACKER_LOCAL_RANKING_H
