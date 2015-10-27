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
//rework  this
class CPingAction : public common::CAction, public  boost::statechart::state_machine< CPingAction, CUninitialised >
{
public:
	CPingAction( CPubKey const & _partnerKey );

	CPingAction( uint256 const & _actionKey, CPubKey const & _partnerKey );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	static bool isPinged( uintptr_t _nodeIndicator );

	CPubKey getPartnerKey()const
	{
		return m_partnerKey;
	}

	~CPingAction(){};
private:
	CPubKey m_partnerKey;
};


}

#endif // PING_ACTION_H
