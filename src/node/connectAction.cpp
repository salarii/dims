// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectAction.h"

#include "setResponseVisitor.h"

#include <boost/assign/list_of.hpp>

namespace node
{

std::vector< TrackerInfo::Enum > const CConnectAction::ms_trackerDescription = boost::assign::list_of< TrackerInfo::Enum >( TrackerInfo::Ip);//.convert_to_container<std::vector< TrackerInfo::Enum > >();

CConnectAction::CConnectAction( State::Enum const _state )
:m_state( _state )
{
}

void
CConnectAction::accept( CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

CRequest*
CConnectAction::execute()
{
	if ( m_state == State::Done )
		return 0;

	if ( m_actionStatus == ActionStatus::InProgress )
	{
		if ( m_state == State::Manual )
		{
			if ( m_trackerInfo )
			{
				m_state = State::Done;

				CTrackerLocalRanking::getInstance()->addTracker( buildTrackerStatsFromData() );
				m_trackerInfo->begin();
			}
			
		}

		assert( m_token );
		return new CInfoRequestContinue( *m_token, RequestKind::NetworkInfo );
	}

	if ( m_state == State::Manual )
	{
		return new CTrackersInfoRequest( ms_trackerDescription );
	}
	else
	{
		// superfluous at  this point
	}

	return 0;
}

void
CConnectAction::setTrackerInfo( boost::optional< std::vector< std::string > > const & _trackerInfo )
{
	m_trackerInfo = _trackerInfo;
}

void
CConnectAction::setInProgressToken( boost::optional< uint256 > const & _trackerInfo )
{
	m_token = _trackerInfo;
}

CTrackerStats
CConnectAction::buildTrackerStatsFromData()
{
	CTrackerStats trackerStats;

	if ( !m_trackerInfo )
		return trackerStats;

	std::vector< std::string > tempInfo = *m_trackerInfo;

	BOOST_REVERSE_FOREACH( TrackerInfo::Enum trackerInfo, ms_trackerDescription )
	{
		assert( !tempInfo.empty() );
		switch( trackerInfo )
		{
		case TrackerInfo::Ip :
			trackerStats.m_ip = QString::fromStdString( tempInfo.back() );
			break;
		case TrackerInfo::Price :
			trackerStats.m_price = QString::fromStdString( tempInfo.back() ).toDouble();
			break;
		case TrackerInfo::Rating :
			trackerStats.m_reputation = QString::fromStdString( tempInfo.back() ).toUInt();
			break;
		case TrackerInfo::publicKey :
			trackerStats.m_publicKey = QString::fromStdString( tempInfo.back() );
			break;
		default:
			break;
		}
		tempInfo.pop_back();
	}
	
	return trackerStats;
}


}



