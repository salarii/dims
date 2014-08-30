// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clientRequests.h"

#include "common/medium.h"

namespace client
{

CInfoRequestContinue::CInfoRequestContinue( uint256 const & _token, common::CMediumFilter< NodeResponses > * _mediumFilter )
	: common::CRequest< NodeResponses >( _mediumFilter )
	, m_token( _token )
{
}

void
CInfoRequestContinue::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< NodeResponses > *
CInfoRequestContinue::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}


}
