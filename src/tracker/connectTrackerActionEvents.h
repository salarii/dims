#ifndef CONNECT_TRACKER_ACTION_EVENTS_H
#define CONNECT_TRACKER_ACTION_EVENTS_H

#include <boost/statechart/event.hpp>

#include "selfNode.h"

namespace tracker
{


struct CNodeConnectedEvent : boost::statechart::event< CNodeConnectedEvent >
{
	CNodeConnectedEvent( CSelfNode * _node ):m_node( _node ){};
	CSelfNode * m_node;
};

struct CRequestedEvent : boost::statechart::event< CRequestedEvent >
{
	CRequestedEvent( CSelfNode * _node ):m_node( _node ){};
	CSelfNode * m_node;
};

struct CErrorEvent : boost::statechart::event< CErrorEvent >
{
};

}

#endif // CONNECT_TRACKER_ACTION_EVENTS_H
