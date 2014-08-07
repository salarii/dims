// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SYNCHRONIZATION_REQUESTS_H
#define SYNCHRONIZATION_REQUESTS_H

#include "common/request.h"
#include "configureTrackerActionHandler.h"

#include <boost/statechart/event.hpp>

namespace tracker
{

struct CDiskBlock;

class CGetSynchronizationInfoRequest : public common::CRequest< TrackerResponses >
{
public:
	CGetSynchronizationInfoRequest( uint256 const & _actionKey, uint64_t _timeStamp );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual int getMediumFilter() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
	uint64_t const m_timeStamp;
};

class CGetNextBlockRequest : public common::CRequest< TrackerResponses >
{
public:
	CGetNextBlockRequest( uint256 const & _actionKey, unsigned int _mediumId );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual int getMediumFilter() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
	unsigned int m_mediumId;
};

class CSetNextBlockRequest : public common::CRequest< TrackerResponses >
{
public:
	CSetNextBlockRequest( uint256 const & _actionKey, unsigned int _mediumId, CDiskBlock * _discBlock );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual int getMediumFilter() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
	unsigned int m_mediumId;
	CDiskBlock * m_discBlock;
};

}

#endif // SYNCHRONIZATION_REQUESTS_H
