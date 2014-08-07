// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectToTrackerRequest.h"
#include "common/medium.h"
#include "common/mediumKinds.h"

namespace tracker
{

CConnectToTrackerRequest::CConnectToTrackerRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress )
	:m_trackerAddress( _trackerAddress )
	,m_serviceAddress( _serviceAddress )
{
}

void
CConnectToTrackerRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CConnectToTrackerRequest::getMediumFilter() const
{
	return common::CMediumKinds::Internal;
}

std::string
CConnectToTrackerRequest::getAddress() const
{
	return m_trackerAddress;
}

CAddress
CConnectToTrackerRequest::getServiceAddress() const
{
	return m_serviceAddress;
}


}
