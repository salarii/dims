// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_LOCAL_RANKING_H
#define TRACKER_LOCAL_RANKING_H

#include <boost/signals2.hpp>

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
		return _tracker1.m_reputation < _tracker2.m_reputation;
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
	virtual std::list< common::CMedium< NodeResponses > *> provideConnection( common::CMediumFilter< NodeResponses > const & _mediumFilter );

	~CTrackerLocalRanking();

	static CTrackerLocalRanking* getInstance();

	void addTracker( common::CTrackerStats const & _trackerStats );

	void addUnidentifiedNode( std::string const & _ip, common::CUnidentifiedNodeInfo const & _unidentifiedNode );

	bool isInUnidentified( std::string const & _ip ) const;

	void removeUnidentifiedNode( std::string const & _ip );

	void addUndeterminedTracker( common::CNodeInfo const & _undeterminedTracker );

	bool getUndeterminedTracker( std::string const & _ip, common::CNodeInfo & _undeterminedTracker );

	void removeUndeterminedTracker( std::string const & _ip );

	void addMonitor( common::CNodeInfo const & _undeterminedTracker );

	void removeMonitor( std::string const & _ip );

	void resetMonitors();

	void resetTrackers();

	std::list< common::CMedium< NodeResponses > *> getMediumByClass( common::RequestKind::Enum _requestKind, unsigned int _mediumNumber );

	common::CMedium< NodeResponses > * getSpecificTracker( uintptr_t _trackerPtr ) const;

	bool isValidMonitorKnown( CKeyID const & _monitorId );

	bool isValidTrackerKnown( CKeyID const & _trackerId );

	bool getTrackerStats( CKeyID const & _trackerId, common::CTrackerStats & _trackerStats );

	bool getSpecificTrackerMedium( CKeyID const & _trackerId, common::CMedium< NodeResponses > *& _medium );

	bool getNodeKey( std::string const & _ip, CPubKey & _pubKey ) const;

	bool getNodeInfo( CPubKey const & _key, common::CNodeInfo & nodeInfo ) const;

	void setIpAndKey( std::string const & _ip, CPubKey const _pubKey );

	unsigned int monitorCount() const;

	unsigned int determinedTrackersCount() const;

	std::vector< common::CTrackerStats > getTrackers() const;

	std::vector< common::CNodeInfo > getMonitors() const;

	void connectNetworkRecognized( boost::signals2::slot< void () > const & _slot );
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

	std::map< uintptr_t, common::CMedium< NodeResponses > * > m_mediumRegister;

	std::map< std::string, common::CUnidentifiedNodeInfo > m_unidentifiedNodes;

	// this  is  definitely not final version
	std::map< CPubKey, common::CNodeInfo > m_monitors;

	std::map< CPubKey, common::CNodeInfo > m_undeterminedTrackers;

	std::map< std::string, CPubKey > m_ipToKey;

	typedef std::pair< std::string, common::CUnidentifiedNodeInfo > Unidentified;

	// this  is  for some  external use
	boost::signals2::signal< void () > m_recognized;
};


template< typename Stats >
common::CMedium< NodeResponses > *
CTrackerLocalRanking::getNetworkConnection( Stats const & _stats )
{
	std::map< std::string, common::CMedium< NodeResponses > * >::iterator iterator = m_createdMediums.find( _stats.m_ip );
	if ( iterator != m_createdMediums.end() )
		return iterator->second;

	common::CMedium< NodeResponses > * medium = static_cast<common::CMedium< NodeResponses > *>( new CNetworkClient( QString::fromStdString( _stats.m_ip ), common::dimsParams().getDefaultClientPort() ) );
	m_createdMediums.insert( std::make_pair( _stats.m_ip, medium ) );
	m_mediumRegister.insert( std::make_pair( common::convertToInt( medium ), medium ) );

	return medium;
}

}

#endif // TRACKER_LOCAL_RANKING_H
