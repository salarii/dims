// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECT_NODE_ACTION_H
#define CONNECT_NODE_ACTION_H

#include "common/scheduleAbleAction.h"
#include "common/filters.h"

#include <boost/statechart/state_machine.hpp>

#include "protocol.h"
/*
current communication protocol is ineffective
consider using ack  request after  every successful message  reception
*/


namespace monitor
{

struct CMonitorConnectNodeActionUninitiated;

class CConnectNodeAction : public common::CScheduleAbleAction, public  boost::statechart::state_machine< CConnectNodeAction, CMonitorConnectNodeActionUninitiated >
{
public:
	CConnectNodeAction( CAddress const & _addrConnect );

	CConnectNodeAction( uint256 const & _actionKey, uintptr_t _nodePtr );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	void setServiceAddress( CAddress const & _addrConnect );

	CAddress getServiceAddress() const;

	std::vector< unsigned char > const & getPayload() const;

	void setNodePtr( uintptr_t _nodePtr );

	uintptr_t getNodePtr() const;

	CPubKey getPublicKey() const;

	void setPublicKey( CPubKey const & _pubKey );

	common::CRole::Enum getRole() const{ return m_role; }

	void setRole( common::CRole::Enum _role ){ m_role = _role; }

	~CConnectNodeAction(){};
private:
	static int const ms_randomPayloadLenght = 32;

	std::vector< unsigned char > m_payload;

	bool const m_passive;

	CAddress m_addrConnect;

	CPubKey m_key;

	uintptr_t m_nodePtr;

	common::CRole::Enum m_role;
};


}

#endif // CONNECT_NODE_ACTION_H
