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

struct CNetworkRecognizedEvent : boost::statechart::event< CNetworkRecognizedEvent >
{
	CNetworkRecognizedEvent(){};
	CNetworkRecognizedEvent( std::set< CValidNodeInfo > const & _trackersInfo, std::set< CValidNodeInfo > const & _monitorsInfo ):m_trackersInfo( _trackersInfo ),m_monitorsInfo( _monitorsInfo ){};
	std::set< CValidNodeInfo > m_trackersInfo;
	std::set< CValidNodeInfo > m_monitorsInfo;
};

struct CNetworkInfoEvent : boost::statechart::event< CNetworkInfoEvent >
{
	CNetworkInfoEvent(){};
	CNetworkInfoEvent( CValidNodeInfo const & _self, common::CRole::Enum _role, std::set< CValidNodeInfo > const & _trackersInfo, std::set< CValidNodeInfo > const & _monitorsInfo ): m_self( _self ), m_role( _role ), m_trackersInfo( _trackersInfo ),m_monitorsInfo( _monitorsInfo ){};
	CValidNodeInfo m_self;
	common::CRole::Enum m_role;
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

struct CRegistrationDataEvent : boost::statechart::event< CRegistrationDataEvent >
{
	CRegistrationDataEvent( CPubKey const & _key, uint64_t _registrationTime, uint64_t const & _period )
		: m_key( _key )
		, m_registrationTime( _registrationTime )
		, m_period( _period )
	{}

	CPubKey m_key;
	uint64_t m_registrationTime;
	uint64_t m_period;
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
	CMerkleBlocksEvent( std::vector< CMerkleBlock > const & _merkles, std::map< uint256 ,std::vector< CTransaction > > const & _transactions, uintptr_t _nodePtr )
		: m_merkles( _merkles ), m_transactions( _transactions ), m_nodePtr( _nodePtr ){};
	std::vector< CMerkleBlock > m_merkles;
	std::map< uint256 ,std::vector< CTransaction > > m_transactions;
	uintptr_t const m_nodePtr;
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


template < class Block >
struct CTransactionBlockEvent : boost::statechart::event< CTransactionBlockEvent<Block> >
{
	CTransactionBlockEvent( Block * _discBlock, unsigned int _blockIndex ):m_discBlock( _discBlock ),m_blockIndex( _blockIndex )
	{
	}
	Block * m_discBlock;
	unsigned int m_blockIndex;
};

struct CSynchronizationInfoEvent : boost::statechart::event< CSynchronizationInfoEvent >
{
	CSynchronizationInfoEvent( uint64_t _timeStamp, unsigned int _nodeIdentifier ):m_timeStamp( _timeStamp ),m_nodeIdentifier(_nodeIdentifier){}

	uint64_t const m_timeStamp;
	unsigned int m_nodeIdentifier;
};

template < class Action >
class CResolveScheduledResult : public boost::static_visitor< void >
{
public:
	CResolveScheduledResult( Action * const _action)
		: m_action( _action )
	{}

	void operator()( CNetworkInfoResult const & _networkInfoResult ) const
	{
		m_action->process_event( common::CNetworkInfoEvent( _networkInfoResult.m_nodeSelfInfo, _networkInfoResult.m_role, _networkInfoResult.m_trackersInfo, _networkInfoResult.m_monitorsInfo ) );
	}

	void operator()( CTransaction const & ) const
	{
	}

	void operator()( CStorageInfo const & _storageInfo ) const
	{
		this->m_action->process_event( _storageInfo );
	}

	void operator()( CValidRegistration const & _validRegistration ) const
	{
		this->m_action->process_event( common::CRegistrationDataEvent( _validRegistration.m_key, _validRegistration.m_registrationTime, _validRegistration.m_period ) );
	}
private:
	Action * m_action;
};


}

#endif // COMMON_EVENTS_H
