// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/action.h"
#include "common/types.h"

#include <boost/statechart/state_machine.hpp>

#ifndef REGISTER_ACTION_H
#define REGISTER_ACTION_H

namespace tracker
{

struct CInitiateRegistration;

class CRegisterAction : public common::CAction< common::CTrackerTypes >, public  boost::statechart::state_machine< CRegisterAction, CInitiateRegistration >, public common::CCommunicationAction
{
public:
	CRegisterAction( uint256 const & _actionKey, uintptr_t _mediumPtr );

	virtual void accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor );

	uintptr_t getMedium() const { return m_medium; }

	~CRegisterAction(){};
private:
	uintptr_t m_medium;
};

}

#endif // REGISTER_ACTION_H
