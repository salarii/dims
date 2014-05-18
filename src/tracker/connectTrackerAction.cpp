// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectTrackerAction.h"
#include "connectToTrackerRequest.h"
#include "common/setResponseVisitor.h"
namespace tracker
{

CConnectTrackerAction::CConnectTrackerAction( std::string const & _trackerAddress )
	:m_trackerAddress( _trackerAddress )
{
}

common::CRequest< TrackerResponses >*
CConnectTrackerAction::execute()
{
	if ( common::ActionStatus::Unprepared == m_actionStatus )
	{
		m_actionStatus = common::ActionStatus::InProgress;
		return new CConnectToTrackerRequest( m_trackerAddress );
	}
	else
	{
		return 0;
	}
}

void
CConnectTrackerAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}

}
