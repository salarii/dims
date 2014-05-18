// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONTROL_TRACKER_EVENTS_H
#define CONTROL_TRACKER_EVENTS_H

namespace tracker
{

struct CGetStateEvent : boost::statechart::event< CGetStateEvent >
{
};

struct CConnectWithTrackerRequest : boost::statechart::event< CConnectWithTrackerRequest >
{
	CConnectWithTrackerRequest( std::string const & _trackerAddress ):m_trackerAddress( _trackerAddress ){};

	std::string const m_trackerAddress;
};

}

#endif // CONTROL_TRACKER_EVENTS_H
