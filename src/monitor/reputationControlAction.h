// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REPUTATION_CONTROL_ACTION_HPP
#define REPUTATION_CONTROL_ACTION_HPP

#include <boost/statechart/state_machine.hpp>

#include "common/action.h"

namespace monitor
{
struct CReputationControlInitial;

class CReputationControlAction : public common::CAction, public  boost::statechart::state_machine< CReputationControlAction, CReputationControlInitial >
{
public:
	//acton  as  singleton  is an abuse,  it  seems  I need  it but it  shows some  weaknes in  action  handling
	static CReputationControlAction *getInstance();

	static CReputationControlAction *createInstance( uint256 const & _actionKey );

	static CReputationControlAction *createInstance();

	void accept( common::CSetResponseVisitor & _visitor );
private:
	CReputationControlAction();

	CReputationControlAction( uint256 const & _actionKey );

	static CReputationControlAction * ms_instance;
};

}

#endif // REPUTATION_CONTROL_ACTION_HPP
