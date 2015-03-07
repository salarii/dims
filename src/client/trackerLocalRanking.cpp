// Copyright (c) 2014-2015 Dims dev-team
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
	m_balancedRanking.insert( _trackerStats ); 
	m_reputationRanking.insert( _trackerStats );
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

void
CTrackerLocalRanking::removeUndeterminedTracker( std::string const & _ip )
{
	CPubKey pubKey;

	if ( getNodeKey( _ip, pubKey ) )
	{
		m_undeterminedTrackers.erase( pubKey );
	}
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
	m_reputationRanking.clear();
	m_balancedRanking.clear();
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

std::list< common::CClientBaseMedium *>
CTrackerLocalRanking::provideConnection( common::CClientMediumFilter const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

std::list< common::CClientBaseMedium *>
CTrackerLocalRanking::getMediumByClass( common::RequestKind::Enum _requestKind, unsigned int _mediumNumber )
{
	std::list< common::CClientBaseMedium *> mediums;

	switch ( _requestKind )
	{
	case common::RequestKind::Unknown:
		if ( m_unidentifiedNodes.begin() != m_unidentifiedNodes.end() )
		{
			BOOST_FOREACH( Unidentified const & stats, m_unidentifiedNodes )
			{
				mediums.push_back( getNetworkConnection( stats.second ) );
			}
		}
		break;
	case common::RequestKind::UndeterminedTrackers:
		if ( m_undeterminedTrackers.begin() != m_undeterminedTrackers.end() )
		{
			BOOST_FOREACH( PAIRTYPE( CPubKey, common::CNodeInfo ) const & stats, m_undeterminedTrackers )
			{
				mediums.push_back( getNetworkConnection( stats.second ) );
			}
		}
	break;
	case common::RequestKind::Monitors:
		if ( m_monitors.begin() != m_monitors.end() )
		{
			BOOST_FOREACH( PAIRTYPE( CPubKey, common::CNodeInfo ) const & stats, m_monitors )
			{
				mediums.push_back( getNetworkConnection( stats.second ) );
			}
		}
	break;
	case common::RequestKind::Transaction:
		if ( m_balancedRanking.begin() != m_balancedRanking.end() )
		{
			BOOST_FOREACH( common::CTrackerStats const & stats, m_balancedRanking )
			{
				mediums.push_back( getNetworkConnection( stats) );
			}
		}
		break;
	case common::RequestKind::TransactionStatus:
	case common::RequestKind::Balance:
		if ( m_reputationRanking.begin() != m_reputationRanking.end() )
		{
			BOOST_FOREACH( common::CTrackerStats const  & stats, m_balancedRanking )
			{
				mediums.push_back( getNetworkConnection( stats ) );
			}
		}
		break;
		break;
	default:
		;
	}
	// there will be  not many  mediums  I belive
	if ( _mediumNumber != -1 && mediums.size() > _mediumNumber )
		mediums.resize( _mediumNumber );
	return mediums;
}

common::CClientBaseMedium *
CTrackerLocalRanking::getSpecificTracker( uintptr_t _trackerPtr ) const
{
	std::map< uintptr_t, common::CClientBaseMedium * >::const_iterator iterator = m_mediumRegister.find( _trackerPtr );

	return iterator != m_mediumRegister.end() ? iterator->second : 0;

}

bool
CTrackerLocalRanking::isValidTrackerKnown( CKeyID const & _trackerId )
{
	BOOST_FOREACH( common::CTrackerStats const & trackerStats, m_reputationRanking )
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

	BOOST_FOREACH( common::CTrackerStats const & trackerStats, m_reputationRanking )
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
	return m_balancedRanking.size();
}

std::vector< common::CTrackerStats >
CTrackerLocalRanking::getTrackers() const
{
	std::vector< common::CTrackerStats > trackers;
	BOOST_FOREACH( common::CTrackerStats const & tackerStat, m_balancedRanking )
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
CTrackerLocalRanking::determineTracker( unsigned int _amount, common::CTrackerStats & _tracker, unsigned int & _fee )const
{
	unsigned int bestFee = -1;

	unsigned int fee;
	BOOST_FOREACH( common::CTrackerStats const & tracker, m_balancedRanking )
	{
		if ( bestFee > tracker.m_price )
		{
			_tracker = tracker;
			bestFee = tracker.m_price;
		}
	}
	_fee = bestFee;

	return bestFee != -1;
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
CTrackerLocalRanking::getSpecificMedium( CKeyID const & _nodeId, common::CClientBaseMedium *& _medium )
{
	if ( isValidTrackerKnown( _nodeId ) )
	{

		BOOST_FOREACH( common::CTrackerStats const & trackerStats, m_reputationRanking )
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
