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
struct CSpecificMediumFilter;
struct CDiskBlock;

class CGetSynchronizationInfoRequest : public common::CRequest< TrackerResponses >
{
public:
	CGetSynchronizationInfoRequest( uint256 const & _actionKey, uint64_t _timeStamp );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual common::CMediumFilter< TrackerResponses > * getMediumFilter() const;

	uint256 getActionKey() const;

	uint64_t getTimeStamp() const;
private:
	uint256 const m_actionKey;

	uint64_t const m_timeStamp;
};

class CSynchronizationAssistanceRequest : public common::CRequest< TrackerResponses >
{
public:
	CSynchronizationAssistanceRequest( uint256 const & _actionKey, CSpecificMediumFilter * _specificMediumFilter );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual common::CMediumFilter< TrackerResponses > * getMediumFilter() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;


};



class CGetNextBlockRequest : public common::CRequest< TrackerResponses >
{
public:
	CGetNextBlockRequest( uint256 const & _actionKey, common::CMediumFilter< TrackerResponses > * _mediumFilter );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual common::CMediumFilter< TrackerResponses > * getMediumFilter() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

class CSetNextBlockRequest : public common::CRequest< TrackerResponses >
{
public:
	CSetNextBlockRequest( uint256 const & _actionKey, common::CMediumFilter< TrackerResponses > * _mediumFilter, CDiskBlock * _discBlock );

	virtual void accept( common::CMedium< TrackerResponses > * _medium ) const;

	virtual common::CMediumFilter< TrackerResponses > * getMediumFilter() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;

	CDiskBlock * m_discBlock;
};

}

#endif // SYNCHRONIZATION_REQUESTS_H
