#ifndef COMMON_EVENTS_H
#define COMMON_EVENTS_H

#include <boost/statechart/event.hpp>
#include "net.h"

namespace common
{

class CSelfNode;

struct CNodeConnectedEvent : boost::statechart::event< CNodeConnectedEvent >
{
	CNodeConnectedEvent( common::CSelfNode * _node ):m_node( _node ){};
	common::CSelfNode * m_node;
};

struct CCantReachNode : boost::statechart::event< CCantReachNode >
{
};

struct CIntroduceEvent : boost::statechart::event< CIntroduceEvent >
{
	CIntroduceEvent( CAddress const & _address ):m_address( _address ){};
	// ugly but let it be
	CAddress m_address;
};

struct CContinueEvent : boost::statechart::event< CContinueEvent >
{
	CContinueEvent( uint256 const & _keyId ):m_keyId( _keyId ){};
	uint256 m_keyId;
};

struct CErrorEvent : boost::statechart::event< CErrorEvent >
{
};

struct CSwitchToConnectedEvent : boost::statechart::event< CSwitchToConnectedEvent >
{
};

struct CSwitchToConnectingEvent : boost::statechart::event< CSwitchToConnectingEvent >
{
};

}

#endif // COMMON_EVENTS_H
