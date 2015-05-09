// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_CONTROLLER_EVENTS_H
#define TRACKER_CONTROLLER_EVENTS_H

#include <boost/statechart/event.hpp>

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

// do I need this
struct CConnectWithTrackerRequest : boost::statechart::event< CConnectWithTrackerRequest >
{
	CConnectWithTrackerRequest( std::string const & _trackerAddress ):m_trackerAddress( _trackerAddress ){};

	std::string const m_trackerAddress;
};

struct CSetScanBitcoinChainProgress : boost::statechart::event< CSetScanBitcoinChainProgress >
{
	CSetScanBitcoinChainProgress( int _blockLeft ):m_blockLeft( _blockLeft ){}

	int m_blockLeft;
};

struct CBitcoinNetworkConnection : boost::statechart::event< CBitcoinNetworkConnection >
{
	CBitcoinNetworkConnection( int _nodesNumber ):m_nodesNumber( _nodesNumber ){}

	int m_nodesNumber;
};

struct CUpdateStatus : boost::statechart::event< CUpdateStatus >
{
};

}

#endif // TRACKER_CONTROLLER_EVENTS_H
