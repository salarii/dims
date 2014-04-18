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
    // in final version it should be  some kind of state machine, fency style, because at the end we  can end up with quite complex code

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
				return 0;
			}

		}
        else if ( m_token )
        {
            return new CInfoRequestContinue( *m_token, RequestKind::NetworkInfo );
        }
        else if ( m_error )
        {
            if ( *m_error == ErrorType::ServiceDenial )
            {
                // for now  simply repeat
                // it is not clear where should I place  code responsible  for such kind of errors
                // is seems that it is much  better  to pass it a little bit higher than  this
                // but since there is no method  to process errors at this point  I will react  where I am
                //
                return new CTrackersInfoRequest( TrackerDescription );
            }
        }
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
CConnectAction::reset()
{
	CAction::reset();

	m_token.reset();

	m_trackerStats.reset();

	m_monitorInfo.reset();

	m_error.reset();

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


void
CConnectAction::setMediumError( boost::optional< ErrorType::Enum > const & _error )
{
    m_error = _error;
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



