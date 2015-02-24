// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/communicationProtocol.h"

#include "monitor/monitorNodeMedium.h"
#include "monitor/monitorRequests.h"

namespace monitor
{

void
CMonitorNodeMedium::add( CConnectCondition const * _request )
{
	common::CConnectCondition connectCondition( _request->getPrice(), _request->getPeriod() );

	common::CMessage message( connectCondition, _request->getActionKey() );

	m_messages.push_back( message );

	m_indexes.push_back( _request->getActionKey() );
}

void
CMonitorNodeMedium::add( CInfoRequest const * _request )
{
	common::CMessage message( common::CInfoRequestData(), _request->getActionKey() );

	m_messages.push_back( message );

	m_indexes.push_back( _request->getActionKey() );

	updateLastRequest( _request->getActionKey(), (common::CRequest< MonitorResponses >*)_request );
}

}
