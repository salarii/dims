// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackerLocalRanking.h"
#include "networkClient.h"
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
CTrackerLocalRanking::addUnidentifiedNode( std::string const & _ip, common::CUnidentifiedStats const & _unidentifiedNode )
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
CTrackerLocalRanking::addUndeterminedTracker( std::string const & _ip, common::CNodeStats const & _undeterminedTracker )
{
	m_undeterminedTrackers.insert( std::make_pair( _ip, _undeterminedTracker ) );
}

bool
CTrackerLocalRanking::getUndeterminedTracker( std::string const & _ip, common::CNodeStats & _undeterminedTracker )
{
	std::map< std::string, common::CNodeStats >::const_iterator iterator = m_undeterminedTrackers.find( _ip );

	if ( iterator == m_undeterminedTrackers.end() )
		return false;

	_undeterminedTracker = iterator->second;

	return true;
}

void
CTrackerLocalRanking::removeUndeterminedTracker( std::string const & _ip )
{
	m_undeterminedTrackers.erase( _ip );
}

void
CTrackerLocalRanking::addMonitor( std::string const & _ip, common::CNodeStats const & _monitor )
{
	m_monitors.insert( std::make_pair( _ip, _monitor ) );
}

void
CTrackerLocalRanking::removeMonitor( std::string const & _ip )
{
	m_monitors.erase( _ip );
}

std::list< common::CMedium< NodeResponses > *>
CTrackerLocalRanking::provideConnection( common::CMediumFilter< NodeResponses > const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

std::list< common::CMedium< NodeResponses > *>
CTrackerLocalRanking::getMediumByClass( common::RequestKind::Enum _requestKind, unsigned int _mediumNumber )
{
	std::list< common::CMedium< NodeResponses > *> mediums;

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
			BOOST_FOREACH( NodeStats const & stats, m_undeterminedTrackers )
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

common::CMedium< NodeResponses > *
CTrackerLocalRanking::getSpecificTracker( uintptr_t _trackerPtr ) const
{
	std::map< uintptr_t, common::CMedium< NodeResponses > * >::const_iterator iterator = m_mediumRegister.find( _trackerPtr );

	return iterator != m_mediumRegister.end() ? iterator->second : 0;

}


float
CTrackerLocalRanking::getPrice()
{
	return 0.0;
}

}
