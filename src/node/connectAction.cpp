// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectAction.h"

#include "setResponseVisitor.h"

namespace node
{


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

	// this  crap serves only  temporary
	if ( m_state == State::Done )
		return 0;

	if ( m_actionStatus == ActionStatus::InProgress )
	{
		if ( m_state == State::Manual )
		{
			if ( m_trackerStats )
			{
				m_state = State::Done;

				CTrackerLocalRanking::getInstance()->addTracker( *m_trackerStats );
			}
			
		}

		assert( m_token );
		return new CInfoRequestContinue( *m_token, RequestKind::NetworkInfo );
	}

	if ( m_state == State::Manual )
	{
		m_actionStatus = ActionStatus::InProgress;
		return new CTrackersInfoRequest( TrackerDescription );
	}
	else
	{
		// superfluous at  this point
	}

	return 0;
}

void
CConnectAction::setTrackerInfo( boost::optional< CTrackerStats > const & _trackerStats )
{
	m_trackerStats = _trackerStats;
}

void
CConnectAction::setInProgressToken( boost::optional< uint256 > const & _trackerInfo )
{
	m_token = _trackerInfo;
}

/* this could be usefull at some  point 

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
*/

}



