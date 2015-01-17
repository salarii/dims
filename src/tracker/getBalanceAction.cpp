// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "getBalanceAction.h"
#include "getBalanceRequest.h"
#include "common/setResponseVisitor.h"
#include "clientRequestsManager.h"

namespace tracker
{

CGetBalanceAction::CGetBalanceAction( uint160 const & _keyId, uint256 const & _hash )
	: m_keyId( _keyId )
	, m_hash( _hash )
	, m_actionStatus( common::ActionStatus::Unprepared )
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
CGetBalanceAction::passBalance( common::CAvailableCoins const & _availableCoins )
{
	CClientRequestsManager::getInstance()->setClientResponse( m_hash, _availableCoins );
}

void
CGetBalanceAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}


}
