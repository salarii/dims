// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/communicationProtocol.h"

#include "monitor/monitorNodeMedium.h"
#include "monitor/monitorRequests.h"

namespace monitor
{

void
CMonitorNodeMedium::add( CRegistrationTerms const * _request )
{
	common::CRegistrationTerms connectCondition( _request->getPrice(), _request->getPeriod() );

	common::CMessage message( connectCondition, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest*)_request );
}

void
CMonitorNodeMedium::add( CInfoRequest const * _request )
{
	common::CMessage message( common::CInfoRequestData(), _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest*)_request );
}

}
