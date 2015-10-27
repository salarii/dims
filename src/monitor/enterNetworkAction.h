// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ENTER_NETWORK_ACTION_H
#define ENTER_NETWORK_ACTION_H

#include <boost/statechart/state_machine.hpp>

#include "common/action.h"

#include "key.h"

namespace monitor
{

struct CEnterNetworkInitial;

class CEnterNetworkAction : public common::CAction, public  boost::statechart::state_machine< CEnterNetworkAction, CEnterNetworkInitial >
{
public:
	CEnterNetworkAction( CPubKey const & _partnerKey );

	CEnterNetworkAction( uint256 const & _actionKey );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	CPubKey getPartnerKey() const
	{
		return m_partnerKey;
	}
private:
	CPubKey m_partnerKey;
};

}

#endif // ENTER_NETWORK_ACTION_H
