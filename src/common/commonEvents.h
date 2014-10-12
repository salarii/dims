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
	CIntroduceEvent( std::vector<unsigned char> const & _payload, std::vector<unsigned char> const & _signed, CPubKey const & _key, CAddress const & _address = CAddress() ):m_payload( _payload ),m_signed( _signed ),m_key( _key ),m_address(_address){};
	std::vector<unsigned char> m_payload;
	std::vector<unsigned char> m_signed;
	CPubKey m_key;
	CAddress m_address;
};

struct CContinueEvent : boost::statechart::event< CContinueEvent >
{
	CContinueEvent( uint256 const & _keyId ):m_keyId( _keyId ){};
	uint256 m_keyId;
};

struct CRoleEvent : boost::statechart::event< CRoleEvent >
{
	CRoleEvent( int _role ):m_role( _role ){};
	int m_role;
};

struct CNetworkInfoEvent : boost::statechart::event< CNetworkInfoEvent >
{
	CNetworkInfoEvent( std::vector< CValidNodeInfo > const & _networkInfo):m_networkInfo( _networkInfo ){};
	std::vector< CValidNodeInfo > m_networkInfo;
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
	CTrackerStatsEvent( unsigned int _reputation, float _price, unsigned int _maxPrice, unsigned int _minPrice, std::string _ip, uintptr_t _nodeIndicator )
		: m_reputation( _reputation )
		, m_price( _price )
		, m_maxPrice( _maxPrice )
		, m_minPrice( _minPrice )
		, m_ip( _ip )
		, m_nodeIndicator( _nodeIndicator ){};

	unsigned int  m_reputation;
	float m_price;
	unsigned int m_maxPrice;
	unsigned int m_minPrice;
	std::string m_ip;
	uintptr_t m_nodeIndicator;
};

struct CConnectConditionEvent : boost::statechart::event< CConnectConditionEvent >
{
	CConnectConditionEvent( unsigned int _price, uint64_t _period ):m_price( _price ), m_period( _period ){}
	unsigned int m_price;
	uint64_t m_period;
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

}

#endif // COMMON_EVENTS_H
