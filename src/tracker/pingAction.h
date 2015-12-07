// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PING_ACTION_H
#define PING_ACTION_H

#include "common/action.h"
#include "common/filters.h"

#include <boost/statechart/state_machine.hpp>

namespace common
{
class CSelfNode;
}

namespace tracker
{

struct CUninitialised;

class CPingAction : public common::CAction, public  boost::statechart::state_machine< CPingAction, CUninitialised >
{
public:
	CPingAction( common::CSelfNode * _node );

	CPingAction( uint256 const & _actionKey );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	~CPingAction(){};
public:
	common::CSelfNode * m_selfNode;
};


}

#endif // PING_ACTION_H
