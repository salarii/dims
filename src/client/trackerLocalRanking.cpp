// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackerLocalRanking.h"
#include "networkClient.h"

#include "base58.h"

namespace client
{

CTrackerLocalRanking * CTrackerLocalRanking::ms_instance = NULL;


CTrackerLocalRanking::CTrackerLocalRanking()
{
}

CTrackerLocalRanking::~CTrackerLocalRanking()
{
	if ( ms_instance )
		delete ms_instance;
	ms_instance = 0;
}

CTrackerLocalRanking*
CTrackerLocalRanking::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CTrackerLocalRanking();
	};
	return ms_instance;
}

void
CTrackerLocalRanking::addTracker( common::CTrackerStats const & _trackerStats )
{
	m_trackers.insert( _trackerStats );
}

void
CTrackerLocalRanking::addUnidentifiedNode( std::string const & _ip, common::CUnidentifiedNodeInfo const & _unidentifiedNode )
{
	m_unidentifiedNodes.insert( std::make_pair( _ip, _unidentifiedNode ) );
}


bool
CTrackerLocalRanking::isInUnidentified( std::string const & _ip ) const
{
	return m_unidentifiedNodes.find( _ip ) != m_unidentifiedNodes.end();
}

void
CTrackerLocalRanking::removeUnidentifiedNode( std::string const & _ip )
{
	m_unidentifiedNodes.erase( _ip );
}

bool
CTrackerLocalRanking::areThereAnyUnidentifiedNode() const
{
	return !m_unidentifiedNodes.empty();
}

void
CTrackerLocalRanking::addUndeterminedTracker( common::CNodeInfo const & _undeterminedTracker )
{
	CPubKey pubKey;

	if ( !getNodeKey( _undeterminedTracker.m_ip, pubKey ) )
	{
		setIpAndKey( _undeterminedTracker.m_ip, _undeterminedTracker.m_key );
		pubKey = _undeterminedTracker.m_key;
	}
	m_undeterminedTrackers.insert( std::make_pair( pubKey, _undeterminedTracker ) );
}

bool
CTrackerLocalRanking::getUndeterminedTracker( std::string const & _ip, common::CNodeInfo & _undeterminedTracker )
{
	CPubKey pubKey;
	getNodeKey( _ip, pubKey );

	std::map< CPubKey, common::CNodeInfo >::const_iterator iterator = m_undeterminedTrackers.find( pubKey );

	if ( iterator == m_undeterminedTrackers.end() )
		return false;

	_undeterminedTracker = iterator->second;

	return true;
}

bool
CTrackerLocalRanking::isInUndeterminedTracker( CPubKey const & _key )const
{
	return m_undeterminedTrackers.find( _key ) != m_undeterminedTrackers.end();
}

void
CTrackerLocalRanking::removeUndeterminedTracker( std::string const & _ip )
{
	CPubKey pubKey;

	if ( getNodeKey( _ip, pubKey ) )
	{
		m_undeterminedTrackers.erase( pubKey );
	}
}

bool
CTrackerLocalRanking::areThereAnyUndeterminedTrackers() const
{
	return !m_undeterminedTrackers.empty();
}


void
CTrackerLocalRanking::addUndeterminedMonitor( common::CNodeInfo const & _undeterminedMonitor )
{
	CPubKey pubKey;

	if ( !getNodeKey( _undeterminedMonitor.m_ip, pubKey ) )
	{
		setIpAndKey( _undeterminedMonitor.m_ip, _undeterminedMonitor.m_key );
		pubKey = _undeterminedMonitor.m_key;
	}
	m_undeterminedMonitors.insert( std::make_pair( pubKey, _undeterminedMonitor ) );
}

bool
CTrackerLocalRanking::getUndeterminedMonitor( std::string const & _ip, common::CNodeInfo & _undeterminedMonitor )
{
	CPubKey pubKey;
	getNodeKey( _ip, pubKey );

	std::map< CPubKey, common::CNodeInfo >::const_iterator iterator = m_undeterminedMonitors.find( pubKey );

	if ( iterator == m_undeterminedMonitors.end() )
		return false;

	_undeterminedMonitor= iterator->second;

	return true;
}

bool
CTrackerLocalRanking::isInUndeterminedMonitor( CPubKey const & _key )const
{
	return m_undeterminedMonitors.find( _key ) != m_undeterminedMonitors.end();
}

void
CTrackerLocalRanking::removeUndeterminedMonitor( std::string const & _ip )
{
	CPubKey pubKey;

	if ( getNodeKey( _ip, pubKey ) )
	{
		m_undeterminedMonitors.erase( pubKey );
	}
}

bool
CTrackerLocalRanking::areThereAnyUndeterminedMonitors() const
{
	return !m_undeterminedMonitors.empty();
}

void
CTrackerLocalRanking::addMonitor( common::CMonitorInfo const & _monitor )
{
	CPubKey pubKey;

	if ( !getNodeKey( _monitor.m_ip, pubKey ) )
	{
		setIpAndKey( _monitor.m_ip, _monitor.m_key );
		pubKey = _monitor.m_key;
	}

	m_monitors.insert( std::make_pair( pubKey, _monitor ) );
}

void
CTrackerLocalRanking::resetMonitors()
{
	m_monitors.clear();
}

void
CTrackerLocalRanking::resetTrackers()
{
	m_undeterminedTrackers.clear();
	m_trackers.clear();
}

void
CTrackerLocalRanking::removeMonitor( std::string const & _ip )
{
	CPubKey pubKey;

	if ( getNodeKey( _ip, pubKey ) )
	{
		m_monitors.erase( pubKey );
	}
}

std::list< common::CMedium *>
CTrackerLocalRanking::provideConnection( common::CMediumFilter const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

struct CompareBalancedTracker
{
	bool operator() ( common::CTrackerStats const & _tracker1, common::CTrackerStats const & _tracker2) const
	{
		return calculateBalanced( _tracker1.m_price, _tracker1.m_reputation * m_modifier )
				< calculateBalanced( _tracker2.m_price, _tracker2.m_reputation * m_modifier );
	}

	double calculateBalanced( double _price, double _reputation )const
	{
		double normalization = -5.3724+3.3381*log( _reputation );
		double price = -5.0505e-03+ 1/( 9.9497e-01 + ( 1.9701e-04 )* _price );
		return normalization * price;
	}
	static double m_modifier;
};

double CompareBalancedTracker::m_modifier = 0;

struct CompareReputationTracker
{
	bool operator() ( common::CTrackerStats const & _tracker1, common::CTrackerStats const & _tracker2) const
	{
		return _tracker1.m_reputation < _tracker2.m_reputation;
	}
};

struct ComparePriceTracker// : public std::binary_function< common::CTrackerStats ,common::CTrackerStats ,bool>
{
	bool operator() ( common::CTrackerStats const & _tracker1, common::CTrackerStats const & _tracker2) const
	{
		return _tracker1.m_price < _tracker2.m_price;
	}
};

std::list< common::CMedium *>
CTrackerLocalRanking::getMediumByClass( ClientMediums::Enum _requestKind, unsigned int _mediumNumber )
{
	std::list< common::CMedium *> mediums;

	if ( _requestKind == ClientMediums::Unknown )
	{
		if ( m_unidentifiedNodes.begin() != m_unidentifiedNodes.end() )
		{
			BOOST_FOREACH( Unidentified const & stats, m_unidentifiedNodes )
			{
				mediums.push_back( getNetworkConnection( stats.second ) );
			}
		}
	}
	else if ( _requestKind == ClientMediums::UndeterminedTrackers )
	{
		if ( m_undeterminedTrackers.begin() != m_undeterminedTrackers.end() )
		{
			BOOST_FOREACH( PAIRTYPE( CPubKey, common::CNodeInfo ) const & stats, m_undeterminedTrackers )
			{
				mediums.push_back( getNetworkConnection( stats.second ) );
			}
		}
	}
	else if ( _requestKind == ClientMediums::UndeterminedMonitors )
	{
		if ( m_undeterminedMonitors.begin() != m_undeterminedMonitors.end() )
		{
			BOOST_FOREACH( PAIRTYPE( CPubKey, common::CNodeInfo ) const & stats, m_undeterminedMonitors )
			{
				mediums.push_back( getNetworkConnection( stats.second ) );
			}
		}
	}
	else if ( _requestKind == ClientMediums::Monitors )
	{
		if ( m_monitors.begin() != m_monitors.end() )
		{
			BOOST_FOREACH( PAIRTYPE( CPubKey, common::CNodeInfo ) const & stats, m_monitors )
			{
				mediums.push_back( getNetworkConnection( stats.second ) );
			}
		}
	}
	else if ( _requestKind == ClientMediums::TrackersPrice )
	{
		std::set< common::CTrackerStats, ComparePriceTracker > priceTrackers;
		BOOST_FOREACH( common::CTrackerStats const & stats, m_trackers )
		{
			priceTrackers.insert( stats );
		}

		BOOST_FOREACH( common::CTrackerStats const & stats, priceTrackers )
		{
			mediums.push_back( getNetworkConnection(stats) );
		}

	}
	else if ( _requestKind == ClientMediums::TrackersRep )
	{
		std::set< common::CTrackerStats, CompareReputationTracker > reputationTrackers;
		BOOST_FOREACH( common::CTrackerStats const & stats, m_trackers )
		{
			reputationTrackers.insert( stats );
		}

		BOOST_FOREACH( common::CTrackerStats const & stats, reputationTrackers )
		{
			mediums.push_back( getNetworkConnection(stats) );
		}

	}
	else if ( _requestKind == ClientMediums::TrackersBalanced )
	{
		double maxRep = 0;
		BOOST_FOREACH( common::CTrackerStats const & stats, m_trackers )
		{
			if ( maxRep < stats.m_reputation )
				maxRep = stats.m_reputation;
		}
		CompareBalancedTracker::m_modifier = 100 / maxRep;

		std::set< common::CTrackerStats, CompareBalancedTracker > balancedTrackers;
		BOOST_FOREACH( common::CTrackerStats const & stats, m_trackers )
		{
			balancedTrackers.insert( stats );
		}

		BOOST_FOREACH( common::CTrackerStats const & stats, balancedTrackers )
		{
			mediums.push_back( getNetworkConnection(stats) );
		}
	}
	// there will be  not many  mediums  I belive
	if ( _mediumNumber != (unsigned int)-1 && mediums.size() > _mediumNumber )
		mediums.resize( _mediumNumber );
	return mediums;
}

common::CMedium *
CTrackerLocalRanking::getSpecificTracker( uintptr_t _trackerPtr ) const
{
	std::map< uintptr_t, common::CMedium * >::const_iterator iterator = m_mediumRegister.find( _trackerPtr );

	return iterator != m_mediumRegister.end() ? iterator->second : 0;

}

bool
CTrackerLocalRanking::isValidTrackerKnown( CKeyID const & _trackerId )
{
	BOOST_FOREACH( common::CTrackerStats const & trackerStats, m_trackers )
	{
		if ( trackerStats.m_key.GetID() == _trackerId )
			return true;
	}

	return false;
}

bool
CTrackerLocalRanking::getTrackerStats( CKeyID const & _trackerId, common::CTrackerStats & _trackerStats )
{
	if ( !isValidTrackerKnown( _trackerId ) )
		return false;

	BOOST_FOREACH( common::CTrackerStats const & trackerStats, m_trackers )
	{
		if ( trackerStats.m_key.GetID() == _trackerId )
		{
			_trackerStats = trackerStats;
			return true;
		}
	}
	assert( !"can't be here" );
	return false;
}

bool
CTrackerLocalRanking::getNodeKey( std::string const & _ip, CPubKey & _pubKey ) const
{
	std::map< std::string, CPubKey >::const_iterator iterator = m_ipToKey.find( _ip );
	if ( iterator != m_ipToKey.end() )
	{
		_pubKey = iterator->second;
		return true;
	}
	else
	{
		return false;
	}
}

bool
CTrackerLocalRanking::getNodeInfo( CPubKey const & _key, common::CNodeInfo & _nodeInfo ) const
{
	{
		std::map< CPubKey, common::CMonitorInfo >::const_iterator iterator = m_monitors.find( _key );

		if ( iterator != m_monitors.end() )
		{
			_nodeInfo = iterator->second;
			return true;
		}
	}
	{
		std::map< CPubKey, common::CNodeInfo >::const_iterator iterator = m_undeterminedTrackers.find( _key );

		if ( iterator != m_undeterminedTrackers.end() )
		{
			_nodeInfo = iterator->second;
			return true;
		}
	}
	return false;
}

void
CTrackerLocalRanking::setIpAndKey( std::string const & _ip, CPubKey const _pubKey )
{
	m_ipToKey.insert( std::make_pair( _ip, _pubKey ) );
}

unsigned int
CTrackerLocalRanking::monitorCount() const
{
	return m_monitors.size();
}


unsigned int
CTrackerLocalRanking::determinedTrackersCount() const
{
	return m_trackers.size();
}

std::vector< common::CTrackerStats >
CTrackerLocalRanking::getTrackers() const
{
	std::vector< common::CTrackerStats > trackers;
	BOOST_FOREACH( common::CTrackerStats const & tackerStat, m_trackers )
	{
		trackers.push_back( tackerStat );
	}
	return trackers;
}

std::vector< common::CMonitorInfo >
CTrackerLocalRanking::getMonitors() const
{
	std::vector< common::CMonitorInfo > monitors;
	BOOST_FOREACH( PAIRTYPE( CPubKey, common::CMonitorInfo ) const & monitor, m_monitors )
	{
		monitors.push_back( monitor.second );
	}

	return monitors;
}

void
CTrackerLocalRanking::connectNetworkRecognized( boost::signals2::slot< void () > const & _slot )
{
	m_recognized.connect( _slot );
}

bool
CTrackerLocalRanking::getMonitorKeyForTracker( CPubKey const & _trackerKey, CPubKey & _monitorKey )
{
	BOOST_FOREACH( PAIRTYPE( CPubKey, common::CMonitorInfo ) const & monitor, m_monitors )
	{
		std::set< CPubKey >::const_iterator iterator = monitor.second.m_trackersKeys.find( _trackerKey );

		if ( iterator != monitor.second.m_trackersKeys.end() )
		{
			_monitorKey = monitor.first;
			return true;
		}
	}
	return false;
}

bool
CTrackerLocalRanking::getSpecificMedium( CKeyID const & _nodeId, common::CMedium *& _medium )
{
	if ( isValidTrackerKnown( _nodeId ) )
	{

		BOOST_FOREACH( common::CTrackerStats const & trackerStats, m_trackers )
		{
			if ( trackerStats.m_key.GetID() == _nodeId )
			{
				assert( m_createdMediums.find( trackerStats.m_ip ) != m_createdMediums.end() );

				_medium = m_createdMediums.find( trackerStats.m_ip )->second;
				return true;
			}
		}
		assert( !"can't be here" );
	}
	else if ( isValidMonitorKnown( _nodeId ) )
	{
		BOOST_FOREACH( PAIRTYPE( CPubKey, common::CNodeInfo ) const & monitor, m_monitors )
		{
			if ( monitor.first.GetID() == _nodeId )
			{
				assert( m_createdMediums.find( monitor.second.m_ip ) != m_createdMediums.end() );

				_medium = m_createdMediums.find( monitor.second.m_ip )->second;
				return true;
			}
		}
		assert( !"can't be here" );
	}
	return false;
}

bool
CTrackerLocalRanking::isValidMonitorKnown( CKeyID const & _monitorId )
{

	//fix
	BOOST_FOREACH( PAIRTYPE( CPubKey, common::CNodeInfo ) const & monitor, m_monitors )
	{
		if ( monitor.second.m_key.GetID() == _monitorId )
			return true;
	}

	return false;
}
}
