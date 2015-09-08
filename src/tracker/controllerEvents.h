// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_CONTROLLER_EVENTS_H
#define TRACKER_CONTROLLER_EVENTS_H

#include <boost/statechart/event.hpp>

#include "key.h"

namespace tracker
{

struct CGetStateEvent : boost::statechart::event< CGetStateEvent >
{
};

struct CConnectedToTrackerEvent : boost::statechart::event< CConnectedToTrackerEvent >
{
	CConnectedToTrackerEvent(){}
};

struct CSynchronizedWithNetworkEvent : boost::statechart::event< CSynchronizedWithNetworkEvent >
{
};

struct CTrackerConnectingEvent : boost::statechart::event< CTrackerConnectingEvent >
{
	CTrackerConnectingEvent(){}
};
// this indicate registration or accept in network
struct CMonitorAcceptEvent : boost::statechart::event< CMonitorAcceptEvent >
{
	CMonitorAcceptEvent( CPubKey const & _monitorKey )
		: m_monitorKey( _monitorKey )
	{
	}
	CPubKey m_monitorKey;
};
// do I need this??
struct CConnectWithTrackerRequest : boost::statechart::event< CConnectWithTrackerRequest >
{
	CConnectWithTrackerRequest( std::string const & _trackerAddress ):m_trackerAddress( _trackerAddress ){};

	std::string const m_trackerAddress;
};

}

#endif // TRACKER_CONTROLLER_EVENTS_H
