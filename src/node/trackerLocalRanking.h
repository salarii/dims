// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_LOCAL_RANKING_H
#define TRACKER_LOCAL_RANKING_H

#include <deque>
#include <set>
#include <map>
#include <QString>
#include <functional> 

#include "common/connectionProvider.h"
#include "common/requestResponse.h"
#include "common/commonResponses.h"

#include "configureNodeActionHadler.h"

namespace client
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

	void addUnidentifiedNode( common::CUnidentifiedStats const & _unidentifiedNode );
private:
	CTrackerLocalRanking();

	template< typename Stats >
	common::CMedium< NodeResponses > * getNetworkConnection( Stats const & _stats );
private:
	static CTrackerLocalRanking * ms_instance;
	// those  sets should be repeatedly rebuild
	std::set< common::CTrackerStats, CompareBalancedTracker > m_balancedRanking;

	std::set< common::CTrackerStats, CompareReputationTracker > m_reputationRanking;

	std::map< std::string, common::CMedium< NodeResponses > * > m_createdMediums;

	std::deque< common::CUnidentifiedStats > m_unidentifiedNodes;

	// monitors???
};


template< typename Stats >
common::CMedium< NodeResponses > *
CTrackerLocalRanking::getNetworkConnection( Stats const & _stats )
{
	std::map< std::string, common::CMedium< NodeResponses > * >::iterator iterator = m_createdMediums.find( _stats.m_ip );
	if ( iterator != m_createdMediums.end() )
		return iterator->second;

	common::CMedium< NodeResponses > * medium = static_cast<common::CMedium< NodeResponses > *>( new CNetworkClient( QString::fromStdString( _stats.m_ip ), _stats.m_port ) );
	m_createdMediums.insert( std::make_pair( _stats.m_ip, medium ) );

	return medium;
}

}

#endif // TRACKER_LOCAL_RANKING_H
