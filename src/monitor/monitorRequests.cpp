// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "monitorRequests.h"
#include "common/medium.h"
#include "common/mediumKinds.h"
#include "filters.h"

namespace monitor
{

CConnectToNodeRequest::CConnectToNodeRequest( std::string const & _address, CAddress const & _serviceAddress )
	: common::CRequest( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_address( _address )
	, m_serviceAddress( _serviceAddress )
{
}

void
CConnectToNodeRequest::accept( common::CMedium * _medium ) const
{
	_medium->add( this );
}

std::string
CConnectToNodeRequest::getAddress() const
{
	return m_address;
}

CAddress
CConnectToNodeRequest::getServiceAddress() const
{
	return m_serviceAddress;
}


}

