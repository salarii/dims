#ifndef CONNECT_TRACKER_ACTION_EVENTS_H
#define CONNECT_TRACKER_ACTION_EVENTS_H

#include <boost/statechart/event.hpp>

#include "net.h"

namespace tracker
{


struct CNodeConnectedEvent : boost::statechart::event< CNodeConnectedEvent >
{
	CNodeConnectedEvent( CNode * _node ):m_node( _node ){};
	CNode * m_node;
};

struct CErrorEvent : boost::statechart::event< CErrorEvent >
{
};

}

#endif // CONNECT_TRACKER_ACTION_EVENTS_H
