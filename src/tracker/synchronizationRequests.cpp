// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "synchronizationRequests.h"
#include "common/medium.h"
#include "common/mediumKinds.h"

namespace tracker
{

CGetSynchronizationInfoRequest::CGetSynchronizationInfoRequest( uint256 const & _actionKey, uint64_t _timeStamp )
	: m_actionKey( _actionKey )
	, m_timeStamp( _timeStamp )
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

CGetNextBlockRequest::CGetNextBlockRequest( uint256 const & _actionKey, unsigned int _mediumId )
	: m_actionKey( _actionKey )
	, m_mediumId( _mediumId )
{
}

void
CGetNextBlockRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CGetNextBlockRequest::getKind() const
{
	return m_mediumId;
}

uint256
CGetNextBlockRequest::getActionKey() const
{
	return m_actionKey;
}

CSetNextBlockRequest::CSetNextBlockRequest( uint256 const & _actionKey, unsigned int _mediumId, CDiskBlock * _discBlock )
	: m_actionKey( _actionKey )
	, m_mediumId( _mediumId )
	, m_discBlock( _discBlock )
{
}

void
CSetNextBlockRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CSetNextBlockRequest::getKind() const
{
	return m_mediumId;
}

uint256
CSetNextBlockRequest::getActionKey() const
{
	return m_actionKey;
}


}
