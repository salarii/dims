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
	: common::CRequest< common::CMonitorTypes >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_address( _address )
	, m_serviceAddress( _serviceAddress )
{
}

void
CConnectToNodeRequest::accept( common::CMonitorBaseMedium * _medium ) const
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

CRegistrationTerms::CRegistrationTerms( unsigned int _price, int64_t const & _period, uint256 const & _actionKey, uint256 const & _id, common::CMonitorMediumFilter * _mediumFilter )
	: common::CRequest< common::CMonitorTypes >( _id, _mediumFilter )
	, m_actionKey( _actionKey )
	, m_price( _price )
	, m_period( _period )
{
}

void
CRegistrationTerms::accept( common::CMonitorBaseMedium * _medium ) const
{
	_medium->add( this );
}

CInfoRequest::CInfoRequest( uint256 const & _actionKey, common::CMonitorMediumFilter * _mediumFilter )
	: common::CRequest< common::CMonitorTypes >( _mediumFilter )
	, m_actionKey( _actionKey )
{
}

void
CInfoRequest::accept( common::CMonitorBaseMedium * _medium ) const
{
	_medium->add( this );
}

}

