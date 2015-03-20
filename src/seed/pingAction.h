// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PING_ACTION_H
#define PING_ACTION_H

#include "common/action.h"
#include "common/filters.h"

#include <boost/statechart/state_machine.hpp>

namespace seed
{
struct CUninitialised;

class CPingAction : public common::CAction< _Type >, public  boost::statechart::state_machine< CPingAction, CUninitialised >, public common::CCommunicationAction
{
public:
	CPingAction( uintptr_t _nodeIndicator, uint256 const & _actionKey );

	virtual void accept( common::CSetResponseVisitor< common::CSeedTypes > & _visitor );

	uintptr_t getNodeIndicator()const;

	~CPingAction(){};
private:
	uintptr_t m_nodeIndicator;
};


}

#endif // PING_ACTION_H
