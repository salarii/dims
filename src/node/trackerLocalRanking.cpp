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

CNetworkClient *
CTrackerLocalRanking::provideConnection( RequestKind::Enum const _actionKind )
{
	return 0;
}

std::list< CNetworkClient *> 
CTrackerLocalRanking::provideConnection( RequestKind::Enum const _actionKind, unsigned _requestedConnectionNumber )
{
	return std::list< CNetworkClient *>();
}

float
CTrackerLocalRanking::getPrice()
{
	return 0.0;
}


}