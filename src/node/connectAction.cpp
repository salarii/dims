// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectAction.h"

#include "sendInfoRequestAction.h"

#include <boost/assign/list_of.hpp>

namespace node
{

CConnectAction::CConnectAction( State::Enum const _state )
:m_state( _state )
{

}

void
CConnectAction::accept( CSetResponseVisitor & _visitor )
{

}

CRequest*
CConnectAction::execute()
{
	if ( m_state == State::Done )
		return 0;

	if ( m_actionStatus == ActionStatus::InProgress )
	{
		return new CInfoRequestContinue( m_token, RequestKind::NetworkInfo );
	}

	if ( m_state == State::Manual )
	{
		return new CTrackersInfoRequest( boost::assign::list_of< TrackerInfo::Enum >( TrackerInfo::Ip).convert_to_container<std::vector< TrackerInfo::Enum > >() );
	}
	else
	{
		// superfluous at  this point
	}

	return 0;
}

}



