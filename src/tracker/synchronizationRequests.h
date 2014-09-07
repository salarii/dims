// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SYNCHRONIZATION_REQUESTS_H
#define SYNCHRONIZATION_REQUESTS_H

#include "common/request.h"
#include "common/medium.h"

#include "configureTrackerActionHandler.h"

#include <boost/statechart/event.hpp>

namespace tracker
{
struct CSpecificMediumFilter;
struct CDiskBlock;

class CGetSynchronizationInfoRequest : public common::CRequest< TrackerResponses >
{
public:
	CGetSynchronizationInfoRequest( uint256 const & _actionKey, uint64_t _timeStamp );

	CGetSynchronizationInfoRequest( uint256 const & _actionKey, uint64_t _timeStamp, common::CMediumFilter< TrackerResponses > * _mediumFilter );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual common::CMediumFilter< TrackerResponses > * getMediumFilter() const;

	uint256 getActionKey() const;

	uint64_t getTimeStamp() const;
private:
	uint256 const m_actionKey;

	uint64_t const m_timeStamp;
};

class CGetNextBlockRequest : public common::CRequest< TrackerResponses >
{
public:
	CGetNextBlockRequest( uint256 const & _actionKey, common::CMediumFilter< TrackerResponses > * _mediumFilter, int _blockKind );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual common::CMediumFilter< TrackerResponses > * getMediumFilter() const;

	uint256 getActionKey() const;

	int getBlockKind() const;
private:
	uint256 const m_actionKey;

	int m_blockKind;
};

template < class Block >
class CSetNextBlockRequest : public common::CRequest< TrackerResponses >
{
public:
	CSetNextBlockRequest( uint256 const & _actionKey, common::CMediumFilter< TrackerResponses > * _mediumFilter, Block * _discBlock, unsigned int _blockIndex );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual common::CMediumFilter< TrackerResponses > * getMediumFilter() const;

	uint256 getActionKey() const;

	Block * getBlock() const;

	unsigned int getBlockIndex() const;
private:
	uint256 const m_actionKey;

	Block * m_discBlock;

	unsigned int m_blockIndex;
};

template < class Block >
CSetNextBlockRequest< Block >::CSetNextBlockRequest( uint256 const & _actionKey, common::CMediumFilter< TrackerResponses > * _mediumFilter, Block * _discBlock, unsigned int _blockIndex )
	: common::CRequest< TrackerResponses >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_discBlock( _discBlock )
	, m_blockIndex( _blockIndex )
{
}

template < class Block >
void
CSetNextBlockRequest< Block >::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

template < class Block >
common::CMediumFilter< TrackerResponses > *
CSetNextBlockRequest< Block >::getMediumFilter() const
{
	return common::CRequest< TrackerResponses >::m_mediumFilter;
}

template < class Block >
uint256
CSetNextBlockRequest< Block >::getActionKey() const
{
	return m_actionKey;
}

template < class Block >
Block *
CSetNextBlockRequest< Block >::getBlock() const
{
	return m_discBlock;
}

template < class Block >
unsigned int
CSetNextBlockRequest< Block >::getBlockIndex() const
{
	return m_blockIndex;
}

}

#endif // SYNCHRONIZATION_REQUESTS_H
