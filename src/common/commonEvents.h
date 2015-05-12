// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COMMON_EVENTS_H
#define COMMON_EVENTS_H

#include <boost/statechart/event.hpp>

#include "net.h"
#include "main.h"

#include "common/commonResponses.h"

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

struct CRoleEvent : boost::statechart::event< CRoleEvent >
{
	CRoleEvent( int _role ):m_role( _role ){};
	int m_role;
};

struct CNetworkInfoEvent : boost::statechart::event< CNetworkInfoEvent >
{
	CNetworkInfoEvent(){};
	CNetworkInfoEvent( std::set< CValidNodeInfo > const & _trackersInfo, std::set< CValidNodeInfo > const & _monitorsInfo ):m_trackersInfo( _trackersInfo ),m_monitorsInfo( _monitorsInfo ){};
	std::set< CValidNodeInfo > m_trackersInfo;
	std::set< CValidNodeInfo > m_monitorsInfo;
};

struct CClientNetworkInfoEvent : boost::statechart::event< CClientNetworkInfoEvent >
{
	CClientNetworkInfoEvent( std::vector< CValidNodeInfo > const & _networkInfo, CPubKey const & _selfKey, int _selfRole ):m_networkInfo( _networkInfo ),m_selfKey( _selfKey ), m_selfRole( _selfRole ), m_ip(), m_nodeIndicator( 0 ){};
	CClientNetworkInfoEvent( std::vector< CValidNodeInfo > const & _networkInfo, CPubKey const & _selfKey, int _selfRole , std::string _ip, uintptr_t _nodeIndicator ):m_networkInfo( _networkInfo ),m_selfKey( _selfKey ), m_selfRole( _selfRole ), m_ip( _ip ), m_nodeIndicator( _nodeIndicator ){};
	std::vector< CValidNodeInfo > m_networkInfo;
	CPubKey m_selfKey;
	int m_selfRole;
	std::string m_ip;
	uintptr_t m_nodeIndicator;
};

struct CTrackerStatsEvent : boost::statechart::event< CTrackerStatsEvent >
{
	CTrackerStatsEvent( unsigned int _reputation, float _price, std::string _ip, uintptr_t _nodeIndicator )
		: m_reputation( _reputation )
		, m_price( _price )
		, m_ip( _ip )
		, m_nodeIndicator( _nodeIndicator ){};

	unsigned int  m_reputation;
	unsigned int m_price;
	std::string m_ip;
	uintptr_t m_nodeIndicator;
};

struct CMonitorStatsEvent : boost::statechart::event< CMonitorStatsEvent >
{
	CMonitorStatsEvent( common::CMonitorData const & _monitorData, std::string _ip, uintptr_t _nodeIndicator )
		: m_monitorData( _monitorData )
		, m_ip( _ip )
		, m_nodeIndicator( _nodeIndicator ){};

	common::CMonitorData m_monitorData;
	std::string m_ip;
	uintptr_t m_nodeIndicator;
};

struct CResultEvent : boost::statechart::event< CResultEvent >
{
	CResultEvent( bool _result ):m_result( _result ){}

	unsigned int m_result;
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

struct CAckEvent : boost::statechart::event< CAckEvent >
{
	CAckEvent(){};
	CAckEvent( uintptr_t _nodePtr ):m_nodePtr( _nodePtr )
	{}

	uintptr_t m_nodePtr;
};

struct CEndEvent : boost::statechart::event< CEndEvent >
{
};


struct CGetEvent : boost::statechart::event< CGetEvent >
{
	CGetEvent( int _type ):m_type(_type){}
	int m_type;
};

struct CStartPingEvent : boost::statechart::event< CStartPingEvent >
{
};

struct CStartPongEvent : boost::statechart::event< CStartPongEvent >
{
};

struct CInitialSynchronizationDoneEvent : boost::statechart::event< CInitialSynchronizationDoneEvent >
{
};

struct CMerkleBlocksEvent : boost::statechart::event< CMerkleBlocksEvent >
{
	CMerkleBlocksEvent( std::vector< CMerkleBlock > const & _merkles, std::map< uint256 ,std::vector< CTransaction > > const & _transactions,long long _id ):m_merkles( _merkles ), m_transactions( _transactions ),m_id( _id ){};
	std::vector< CMerkleBlock > m_merkles;
	std::map< uint256 ,std::vector< CTransaction > > m_transactions;
	long long const m_id;
};

struct CUpdateStatus : boost::statechart::event< CUpdateStatus >
{
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

}

#endif // COMMON_EVENTS_H
