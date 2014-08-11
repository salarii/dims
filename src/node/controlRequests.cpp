// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "controlRequests.h"

#include "common/medium.h"
#include "clientFilters.h"

namespace client
{

CDnsInfoRequest::CDnsInfoRequest()
	:common::CRequest< NodeResponses >( new CMediumClassFilter( common::RequestKind::Seed ) )
{
}

void
CDnsInfoRequest::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< NodeResponses > *
CDnsInfoRequest::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}

CRecognizeNetworkRequest::CRecognizeNetworkRequest()
	:common::CRequest< NodeResponses >( new CMediumClassFilter( common::RequestKind::Unknown ) )
{

}

void
CRecognizeNetworkRequest::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< NodeResponses > *
CRecognizeNetworkRequest::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}


}
