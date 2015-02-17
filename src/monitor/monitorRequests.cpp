// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "monitorRequests.h"
#include "common/medium.h"
#include "common/mediumKinds.h"
#include "filters.h"

namespace monitor
{

CConnectToNodeRequest::CConnectToNodeRequest( std::string const & _address, CAddress const & _serviceAddress )
	: common::CRequest< MonitorResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_address( _address )
	, m_serviceAddress( _serviceAddress )
{
}

void
CConnectToNodeRequest::accept( common::CMedium< MonitorResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< MonitorResponses > *
CConnectToNodeRequest::getMediumFilter() const
{
	return m_mediumFilter;
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

CConnectCondition::CConnectCondition( uint256 const & _actionKey,  unsigned int _price, uint256 const & _period, common::CMediumFilter< MonitorResponses > * _mediumFilter )
	: common::CRequest< MonitorResponses >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_price( _price )
	, m_period( _period )
{
}

void
CConnectCondition::accept( common::CMedium< MonitorResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< MonitorResponses > *
CConnectCondition::getMediumFilter() const
{
	return m_mediumFilter;
}

CInfoRequest::CInfoRequest( uint256 const & _actionKey, common::CMediumFilter< MonitorResponses > * _mediumFilter )
	: common::CRequest< MonitorResponses >( _mediumFilter )
	, m_actionKey( _actionKey )
{
}

void
CInfoRequest::accept( common::CMedium< MonitorResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< MonitorResponses > *
CInfoRequest::getMediumFilter() const
{
	return m_mediumFilter;
}

}

