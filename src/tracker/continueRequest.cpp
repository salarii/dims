// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "continueRequest.h"
#include "common/medium.h"

namespace tracker
{

CContinueReqest::CContinueReqest( uint256 const & _id, unsigned int _kind )
	: m_id( _id )
	, m_kind( _kind )
{
}

void
CContinueReqest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CContinueReqest::getKind() const
{
	return m_kind;
}

uint256
CContinueReqest::getRequestId() const
{
	return m_id;
}

}
