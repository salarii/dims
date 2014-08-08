// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "getBalanceRequest.h"
#include "common/medium.h"
#include "common/mediumKinds.h"
#include "common/filters.h"

namespace tracker
{

CGetBalanceRequest::CGetBalanceRequest( uint160 const & _key )
	: m_key( _key )
{
	m_mediumFilter = new common::CMediumFilter< TrackerResponses >( common::CMediumKinds::Internal );
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

CGetBalanceRequest::~CGetBalanceRequest()
{
	delete m_mediumFilter;
}

}
