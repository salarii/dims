// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SYNCHRONIZATION_ACTION_H
#define SYNCHRONIZATION_ACTION_H

#include "common/scheduleAbleAction.h"

#include <boost/statechart/state_machine.hpp>

namespace monitor
{

struct CUninitiatedSynchronization;

class CSynchronizationAction : public common::CScheduleAbleAction, public  boost::statechart::state_machine< CSynchronizationAction, CUninitiatedSynchronization >
{
public:
	CSynchronizationAction( CPubKey const & _partnerKey );

	CSynchronizationAction( uint256 const & _id, uint256 const & _actionKey, CPubKey const & _partnerKey );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	void clear();

	bool isRequestInitialized() const;

	uint256 getRequestKey() const { return m_requestKey; }

	CPubKey getPartnerKey()const
	{
		return m_partnerKey;
	}

	void setRequestKey( uint256 const & _requestKey ){ m_requestKey = _requestKey; }

	~CSynchronizationAction();

private:
	uint256 m_requestKey;

	CPubKey m_partnerKey;
};

}

#endif // SYNCHRONIZATION_ACTION_H
