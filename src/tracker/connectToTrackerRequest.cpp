// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectToTrackerRequest.h"
#include "common/medium.h"
#include "common/mediumKinds.h"
#include "trackerFilters.h"

namespace tracker
{

CConnectToTrackerRequest::CConnectToTrackerRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress )
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_trackerAddress( _trackerAddress )
	, m_serviceAddress( _serviceAddress )
{
}

void
CConnectToTrackerRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< TrackerResponses > *
CConnectToTrackerRequest::getMediumFilter() const
{
	return m_mediumFilter;
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
