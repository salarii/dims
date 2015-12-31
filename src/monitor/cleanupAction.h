// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLEANUP_ACTION_H
#define CLEANUP_ACTION_H

#include "common/action.h"

#include <boost/statechart/state_machine.hpp>

namespace monitor
{

struct CleanupInit;

class CCleanupAction : public common::CAction, public boost::statechart::state_machine< CCleanupAction, CleanupInit >
{
public:
	CCleanupAction();

	virtual void accept( common::CSetResponseVisitor & _visitor );
public:

};

}

#endif // CLEANUP_ACTION_H
