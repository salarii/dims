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
CTrackerLocalRanking::addUnidentifiedNode( common::CUnidentifiedStats const & _unidentifiedNode )
{
	m_unidentifiedNodes.insert( _unidentifiedNode );
}

void
CTrackerLocalRanking::removeUnidentifiedNode( common::CUnidentifiedStats const & _unidentifiedNode )
{
	m_unidentifiedNodes.erase( _unidentifiedNode );
}

void
CTrackerLocalRanking::addUndeterminedTracker( common::CNodeStatistic const & _undeterminedTracker )
{
	m_undeterminedTrackers.insert( _undeterminedTracker );
}

void
CTrackerLocalRanking::removeUndeterminedTracker( common::CNodeStatistic const & _undeterminedTracker )
{
	m_undeterminedTrackers.erase( _undeterminedTracker );
}

void
CTrackerLocalRanking::addMonitor( common::CNodeStatistic const & _monitor )
{
	m_monitors.insert( _monitor );
}

void
CTrackerLocalRanking::removeMonitor( common::CNodeStatistic const & _monitor )
{
	m_monitors.erase( _monitor );
}

std::list< common::CMedium< NodeResponses > *>
CTrackerLocalRanking::provideConnection( int const _actionKind, unsigned _requestedConnectionNumber)
{
	std::list< common::CMedium< NodeResponses > *> mediums;

	switch (_actionKind)
	{
	case common::RequestKind::Unknown:
		if ( m_unidentifiedNodes.begin() != m_unidentifiedNodes.end() )
		{
			BOOST_FOREACH( common::CUnidentifiedStats const & stats, m_unidentifiedNodes )
			{
				mediums.push_back( getNetworkConnection( stats ) );
			}
		}
		break;
	case common::RequestKind::Transaction:
		if ( m_balancedRanking.begin() != m_balancedRanking.end() )
		{
			BOOST_FOREACH( common::CTrackerStats const & stats, m_balancedRanking )
			{
				mediums.push_back( getNetworkConnection( stats ) );
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
	if ( _requestedConnectionNumber != -1 && mediums.size() > _requestedConnectionNumber )
		mediums.resize( _requestedConnectionNumber );
	return mediums;
}

float
CTrackerLocalRanking::getPrice()
{
	return 0.0;
}

}
