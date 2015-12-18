// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CHECK_NETWORK_ACTION_H
#define CHECK_NETWORK_ACTION_H


#include <boost/statechart/state_machine.hpp>

#include "common/action.h"
#include "common/request.h"
#include "configureClientActionHadler.h"
#include "common/setResponseVisitor.h"

#include  <boost/optional.hpp>
#include "uint256.h"
#include "coins.h"

namespace client
{

struct CInitiateNetworkCheck;

class CCheckNetworkAction : public common::CAction, public  boost::statechart::state_machine< CCheckNetworkAction, CInitiateNetworkCheck >
{
public:
	CCheckNetworkAction( bool _autoDelete );

	void accept( common::CSetResponseVisitor & _visitor );

	void reset();
};

}

#endif // CHECK_NETWORK_ACTION_H
