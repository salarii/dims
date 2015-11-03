// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ADMIT_TRACKER_ACTION_H
#define ADMIT_TRACKER_ACTION_H

#include "common/action.h"

#include <boost/statechart/state_machine.hpp>

namespace monitor
{

struct CAdmitInitial;

class CAdmitTrackerAction : public common::CAction, public boost::statechart::state_machine< CAdmitTrackerAction, CAdmitInitial >
{
public:
	CAdmitTrackerAction( CPubKey const & _partnerKey );

	CAdmitTrackerAction( uint256 const & _actionKey, CPubKey const & _partnerKey );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	CPubKey getPartnerKey()const
	{
		return m_partnerKey;
	}

	~CAdmitTrackerAction(){};
private:
	CPubKey m_partnerKey;
};

}

#endif // ADMIT_TRACKER_ACTION_H
