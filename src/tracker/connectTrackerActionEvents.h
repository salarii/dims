// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECT_TRACKER_ACTION_EVENTS_H
#define CONNECT_TRACKER_ACTION_EVENTS_H

#include <boost/statechart/event.hpp>

#include "selfNode.h"

namespace tracker
{

struct CSwitchToConnectingEvent : boost::statechart::event< CSwitchToConnectingEvent >
{
};

struct CSwitchToConnectedEvent : boost::statechart::event< CSwitchToConnectedEvent >
{
};


struct CNodeConnectedEvent : boost::statechart::event< CNodeConnectedEvent >
{
	CNodeConnectedEvent( CSelfNode * _node ):m_node( _node ){};
	CSelfNode * m_node;
};

struct CIntroduceEvent : boost::statechart::event< CIntroduceEvent >
{
	CIntroduceEvent(  );
};

struct CContinueEvent : boost::statechart::event< CContinueEvent >
{
	CContinueEvent( uint256 const & _keyId ):m_keyId( _keyId ){};
	uint256 m_keyId;
};

struct CErrorEvent : boost::statechart::event< CErrorEvent >
{
};

}

#endif // CONNECT_TRACKER_ACTION_EVENTS_H
