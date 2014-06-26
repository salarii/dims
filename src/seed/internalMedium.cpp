// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "internalMedium.h"
#include "common/mediumRequests.h"
#include "common/manageNetwork.h"

#include <algorithm>

namespace seed
{

CInternalMedium * CInternalMedium::ms_instance = NULL;

CInternalMedium*
CInternalMedium::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CInternalMedium();
	};
	return ms_instance;
}

CInternalMedium::CInternalMedium()
{
}

bool
CInternalMedium::serviced() const
{
	return true;
}

void
CInternalMedium::add( common::CConnectToNodeRequest< SeedResponses > const *_request )
{
// in general  it is to slow to be  handled  this  way, but  as usual we can live with that for a while
	common::CSelfNode* node = common::CManageNetwork::getInstance()->connectNode( _request->getServiceAddress(), _request->getAddress().empty()? 0 : _request->getAddress().c_str() );

	m_responses.push_back( common::CConnectedNode( node ) );
}


bool
CInternalMedium::getResponse( std::vector< SeedResponses > & _requestResponse ) const
{
	_requestResponse = m_responses;
	return true;
}

void
CInternalMedium::clearResponses()
{
	m_responses.clear();
}


}

