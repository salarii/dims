// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "getBalanceRequest.h"
#include "common/medium.h"
#include "common/mediumKinds.h"
#include "trackerFilters.h"

namespace tracker
{

CGetBalanceRequest::CGetBalanceRequest( uint160 const & _key )
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_key( _key )
{
}

void
CGetBalanceRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< TrackerResponses > *
CGetBalanceRequest::getMediumFilter() const
{
	return m_mediumFilter;
}

uint160
CGetBalanceRequest::getKey() const
{
	return m_key;
}

}
