// Copyright (c) 2014-2015 DiMS dev-team
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
#include "common/responses.h"
#include "common/responses.h"

#include "client/struct.h"

namespace client
{
/*
y = -5.3724+3.3381*log(x); //reputation  normalized  to  100
y = -5.0505e-03+ 1./( 9.9497e-01 .+   1.9701e-04.*x ); // price  not  normalized
*/

class CNetworkClient;

class CTrackerLocalRanking : public common::CConnectionProvider
{
public:
	virtual std::list< common::CMedium *> provideConnection( common::CMediumFilter const & _mediumFilter );

	~CTrackerLocalRanking();

	static CTrackerLocalRanking* getInstance();

	void addTracker( common::CTrackerStats const & _trackerStats );

	void addUnidentifiedNode( std::string const & _ip, common::CUnidentifiedNodeInfo const & _unidentifiedNode );

	unsigned int getUnidentifiedNodeAmount() const{ return m_unidentifiedNodes.size(); }

	bool isInUnidentified( std::string const & _ip ) const;

	void removeUnidentifiedNode( std::string const & _ip );

	void addUndeterminedTracker( common::CNodeInfo const & _undeterminedTracker );

	bool getUndeterminedTracker( std::string const & _ip, common::CNodeInfo & _undeterminedTracker );

	void removeUndeterminedTracker( std::string const & _ip );

	void addMonitor( common::CMonitorInfo const & _monitor );

	void removeMonitor( std::string const & _ip );

	void resetMonitors();

	void resetTrackers();

	std::list< common::CMedium *> getMediumByClass( ClientMediums::Enum _requestKind, unsigned int _mediumNumber );

	common::CMedium * getSpecificTracker( uintptr_t _trackerPtr ) const;

	bool isValidMonitorKnown( CKeyID const & _monitorId );

	bool isValidTrackerKnown( CKeyID const & _trackerId );

	bool getTrackerStats( CKeyID const & _trackerId, common::CTrackerStats & _trackerStats );

	bool getSpecificMedium( CKeyID const & _trackerId, common::CMedium *& _medium );

	bool getNodeKey( std::string const & _ip, CPubKey & _pubKey ) const;

	bool getNodeInfo( CPubKey const & _key, common::CNodeInfo & nodeInfo ) const;

	void setIpAndKey( std::string const & _ip, CPubKey const _pubKey );

	unsigned int monitorCount() const;

	unsigned int determinedTrackersCount() const;

	std::vector< common::CTrackerStats > getTrackers() const;

	std::vector< common::CMonitorInfo > getMonitors() const;

	void connectNetworkRecognized( boost::signals2::slot< void () > const & _slot );

	bool determineTracker( unsigned int _amount, common::CTrackerStats & _tracker, unsigned int & _fee ) const;// rather  it is not what I really need

	bool getMonitorKeyForTracker( CPubKey const & _trackerKey, CPubKey & _monitorKey );

private:
	CTrackerLocalRanking();

	template< typename Stats >
	common::CMedium * getNetworkConnection( Stats const & _stats );
private:
	static CTrackerLocalRanking * ms_instance;
	std::set< common::CTrackerStats > m_trackers;

	std::map< std::string, common::CMedium * > m_createdMediums;

	std::map< uintptr_t, common::CMedium * > m_mediumRegister;

	std::map< std::string, common::CUnidentifiedNodeInfo > m_unidentifiedNodes;

	// this  is  definitely not final version
	std::map< CPubKey, common::CMonitorInfo > m_monitors;

	std::map< CPubKey, common::CNodeInfo > m_undeterminedTrackers;

	std::map< std::string, CPubKey > m_ipToKey;

	typedef std::pair< std::string, common::CUnidentifiedNodeInfo > Unidentified;

	// this  is  for some  external use
	boost::signals2::signal< void () > m_recognized;
};


template< typename Stats >
common::CMedium *
CTrackerLocalRanking::getNetworkConnection( Stats const & _stats )
{
	std::map< std::string, common::CMedium * >::iterator iterator = m_createdMediums.find( _stats.m_ip );
	if ( iterator != m_createdMediums.end() )
		return iterator->second;

	common::CMedium * medium = static_cast<common::CMedium *>( new CNetworkClient( QString::fromStdString( _stats.m_ip ), common::dimsParams().getDefaultClientPort() ) );
	m_createdMediums.insert( std::make_pair( _stats.m_ip, medium ) );
	m_mediumRegister.insert( std::make_pair( common::convertToInt( medium ), medium ) );

	return medium;
}


}

#endif // TRACKER_LOCAL_RANKING_H
