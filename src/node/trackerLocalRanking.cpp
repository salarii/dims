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

std::list< common::CMedium< NodeResponses > *>
CTrackerLocalRanking::provideConnection( int const _actionKind, unsigned _requestedConnectionNumber)
{
	std::list< common::CMedium< NodeResponses > *> mediums;

	switch (_actionKind)
	{
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

common::CMedium< NodeResponses > *
CTrackerLocalRanking::getNetworkConnection( common::CTrackerStats const & _trackerStats )
{
	std::map< std::string, common::CMedium< NodeResponses > * >::iterator iterator = m_createdMediums.find( _trackerStats.m_publicKey );
    if ( iterator != m_createdMediums.end() )
        return iterator->second;

	common::CMedium< NodeResponses > * medium = static_cast<common::CMedium< NodeResponses > *>( new CNetworkClient( QString::fromStdString( _trackerStats.m_ip ), _trackerStats.m_port ) );
    m_createdMediums.insert( std::make_pair( _trackerStats.m_publicKey, medium ) );

    return medium;

}

}
