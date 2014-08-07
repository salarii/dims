// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "controlRequests.h"
#include "common/medium.h"

namespace client
{

CDnsInfoRequest::CDnsInfoRequest()
{
}

void
CDnsInfoRequest::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

int
CDnsInfoRequest::getMediumFilter() const
{
	return common::RequestKind::Seed;
}

CRecognizeNetworkRequest::CRecognizeNetworkRequest()
{

}

void
CRecognizeNetworkRequest::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

int
CRecognizeNetworkRequest::getMediumFilter() const
{
	return common::RequestKind::Unknown;
}


}
