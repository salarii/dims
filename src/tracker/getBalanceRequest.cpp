// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "getBalanceRequest.h"
#include "common/medium.h"
namespace tracker
{

CGetBalanceRequest::CGetBalanceRequest( uint160 const & _key )
	: m_key( _key )
{
}

void CGetBalanceRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CGetBalanceRequest::getKind()
{
	return 0;
}

uint160
CGetBalanceRequest::getKey() const
{
	return m_key;
}

}
