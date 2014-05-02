// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "getBalanceAction.h"
#include "getBalanceRequest.h"
#include "common/setResponseVisitor.h"
namespace tracker
{

CGetBalanceAction::CGetBalanceAction( uint160 const & _keyId )
	: m_keyId( _keyId )
{
}

common::CRequest< TrackerResponses >*
CGetBalanceAction::execute()
{
	if ( common::ActionStatus::Unprepared == m_actionStatus )
	{
		m_actionStatus = common::ActionStatus::InProgress;
		return new CGetBalanceRequest( m_keyId );
	}
	else
	{
		return 0;
	}

}


void
CGetBalanceAction::passBalance()
{
}

void
CGetBalanceAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}


}
