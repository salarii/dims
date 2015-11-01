// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/medium.h"
#include "common/actionHandler.h"

#include "tracker/trackerNodeMedium.h"
#include "tracker/trackerNodesManager.h"
#include "tracker/connectNodeAction.h"
#include "tracker/requests.h"

namespace tracker
{

void
CTrackerNodeMedium::add( CPassMessageRequest const * _request )
{
	common::CMessage message( _request->getMessage(), _request->getPreviousKey(), _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest*)_request );
}

void
CTrackerNodeMedium::add( CDeliverInfoRequest const * _request )
{
	common::CMessage message( common::CInfoResponseData(), _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest*)_request );
}

}
