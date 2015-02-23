// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "internalOperationsMedium.h"
#include "common/manageNetwork.h"
#include "monitorRequests.h"

#include <algorithm>

namespace monitor
{

CInternalOperationsMedium * CInternalOperationsMedium::ms_instance = NULL;

CInternalOperationsMedium*
CInternalOperationsMedium::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CInternalOperationsMedium();
	};
	return ms_instance;
}

CInternalOperationsMedium::CInternalOperationsMedium()
{
}

bool
CInternalOperationsMedium::serviced() const
{
	return true;
}

void
CInternalOperationsMedium::add( CConnectToNodeRequest const *_request )
{
// in general  it is to slow to be  handled  this  way, but  as usual we can live with that for a while
	common::CSelfNode* node = common::CManageNetwork::getInstance()->connectNode( _request->getServiceAddress(), _request->getAddress().empty()? 0 : _request->getAddress().c_str() );

	m_responses.push_back( common::CConnectedNode( node ) );
}

void
CInternalOperationsMedium::add( common::CContinueReqest< MonitorResponses > const * _request )
{
	m_responses.push_back( common::CContinueResult( 0 ) );
}


bool
CInternalOperationsMedium::getResponseAndClear( std::vector< PAIRTYPE( common::CRequest< MonitorResponses >*, std::vector< MonitorResponses > ) > & _requestResponse )
{
	_requestResponse = m_responses;
	clearResponses();
	return true;
}

void
CInternalOperationsMedium::clearResponses()
{
	m_responses.clear();
}


}
