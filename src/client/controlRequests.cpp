// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "controlRequests.h"

#include "common/medium.h"
#include "filters.h"

namespace client
{

CDnsInfoRequest::CDnsInfoRequest()
	:common::CRequest( new CMediumClassFilter( ClientMediums::Seed ) )
{
}

void
CDnsInfoRequest::accept( common::CMedium * _medium ) const
{
	_medium->add( this );
}

}
