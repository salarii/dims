// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "synchronizationRequests.h"
#include "common/medium.h"
#include "common/mediumKinds.h"

namespace tracker
{

CGetSynchronizationInfoRequest::CGetSynchronizationInfoRequest( uint256 const & _actionKey )
	: m_actionKey( _actionKey )
{

}

void
CGetSynchronizationInfoRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CGetSynchronizationInfoRequest::getKind() const
{
	return common::CMediumKinds::DimsNodes;
}

uint256
CGetSynchronizationInfoRequest::getActionKey() const
{
	return m_actionKey;
}

}
