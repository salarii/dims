// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PING_ACTION_H
#define PING_ACTION_H

#include "common/action.h"
#include "common/filters.h"

#include <boost/statechart/state_machine.hpp>

namespace tracker
{
struct CUninitialised;

class CPingAction : public common::CAction, public  boost::statechart::state_machine< CPingAction, CUninitialised >
{
public:
	CPingAction( uintptr_t _nodeIndicator );

	CPingAction( uint256 const & _actionKey, uintptr_t _nodeIndicator );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	uintptr_t getNodeIndicator() const;

	static bool isPinged( uintptr_t _nodeIndicator );

	~CPingAction(){};
private:
	uintptr_t m_nodeIndicator;

	static std::set< uintptr_t > m_pingedNodes; //a bit ugly
};


}

#endif // PING_ACTION_H
