// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "controlRequests.h"

#include "common/medium.h"
#include "clientFilters.h"

namespace client
{

CDnsInfoRequest::CDnsInfoRequest()
	:common::CRequest< ClientResponses >( new CMediumClassFilter( common::RequestKind::Seed ) )
{
}

void
CDnsInfoRequest::accept( common::CMedium< ClientResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< ClientResponses > *
CDnsInfoRequest::getMediumFilter() const
{
	return common::CRequest< ClientResponses >::m_mediumFilter;
}

CRecognizeNetworkRequest::CRecognizeNetworkRequest()
	:common::CRequest< ClientResponses >( new CMediumClassFilter( common::RequestKind::Unknown ) )
{

}

void
CRecognizeNetworkRequest::accept( common::CMedium< ClientResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< ClientResponses > *
CRecognizeNetworkRequest::getMediumFilter() const
{
	return common::CRequest< ClientResponses >::m_mediumFilter;
}


}
