// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "synchronizationRequests.h"
#include "common/medium.h"
#include "common/mediumKinds.h"
#include "trackerFilters.h"

namespace tracker
{

CGetSynchronizationInfoRequest::CGetSynchronizationInfoRequest( uint256 const & _actionKey, uint64_t _timeStamp )
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Trackers ) )
	, m_actionKey( _actionKey )
	, m_timeStamp( _timeStamp )
{
}

void
CGetSynchronizationInfoRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< TrackerResponses > *
CGetSynchronizationInfoRequest::getMediumFilter() const
{
	return common::CRequest< TrackerResponses >::m_mediumFilter;
}

uint256
CGetSynchronizationInfoRequest::getActionKey() const
{
	return m_actionKey;
}

uint64_t
CGetSynchronizationInfoRequest::getTimeStamp() const
{
	return m_timeStamp;
}

CGetNextBlockRequest::CGetNextBlockRequest( uint256 const & _actionKey, common::CMediumFilter< TrackerResponses > * _mediumFilter )
	: common::CRequest< TrackerResponses >( _mediumFilter )
	, m_actionKey( _actionKey )
{
}

void
CGetNextBlockRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< TrackerResponses > *
CGetNextBlockRequest::getMediumFilter() const
{
	return common::CRequest< TrackerResponses >::m_mediumFilter;
}

uint256
CGetNextBlockRequest::getActionKey() const
{
	return m_actionKey;
}

CSetNextBlockRequest::CSetNextBlockRequest( uint256 const & _actionKey, common::CMediumFilter< TrackerResponses > * _mediumFilter, CDiskBlock * _discBlock )
	: common::CRequest< TrackerResponses >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_discBlock( _discBlock )
{
}

void
CSetNextBlockRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< TrackerResponses > *
CSetNextBlockRequest::getMediumFilter() const
{
	return common::CRequest< TrackerResponses >::m_mediumFilter;
}

uint256
CSetNextBlockRequest::getActionKey() const
{
	return m_actionKey;
}


}
