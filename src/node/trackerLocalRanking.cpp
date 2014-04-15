#include "trackerLocalRanking.h"

namespace node
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
CTrackerLocalRanking::addTracker( CTrackerStats const & _trackerStats )
{
	m_balancedRanking.insert( _trackerStats ); 
	m_reputationRanking.insert( _trackerStats );
}

CMedium *
CTrackerLocalRanking::provideConnection( RequestKind::Enum const _actionKind )
{
/*	switch (_actionKind)
	{
	case RequestKind::Transaction:
		return m_balancedRanking.begin() != m_balancedRanking.end() ? *m_balancedRanking.begin() : 0;
	case RequestKind::TransactionStatus:
		return m_reputationRanking.begin() != m_reputationRanking.end() ? *m_reputationRanking.begin() : 0;
	case RequestKind::Balance:
		return m_reputationRanking.begin() != m_reputationRanking.end() ? *m_reputationRanking.begin() : 0;
	default:*/
		return 0;
	//}
}

std::list< CMedium *> 
CTrackerLocalRanking::provideConnection( RequestKind::Enum const _actionKind, unsigned _requestedConnectionNumber )
{
	return std::list< CMedium *>();
}

float
CTrackerLocalRanking::getPrice()
{
	return 0.0;
}


}