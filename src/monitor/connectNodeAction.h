// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECT_NODE_ACTION_H
#define CONNECT_NODE_ACTION_H

#include "common/action.h"
#include "common/filters.h"

#include "configureMonitorActionHandler.h"

#include <boost/statechart/state_machine.hpp>

#include "protocol.h"
/*
current communication protocol is ineffective
consider using ack  request after  every successful message  reception
*/


namespace monitor
{

struct CMonitorConnectNodeActionUninitiated;

class CConnectNodeAction : public common::CAction< common::CMonitorTypes >, public  boost::statechart::state_machine< CConnectNodeAction, CMonitorConnectNodeActionUninitiated >, public common::CCommunicationAction
{
public:
	CConnectNodeAction( CAddress const & _addrConnect );

	CConnectNodeAction( CAddress const & _addrConnect, uint256 const & _actionKey, uintptr_t _mediumPtr );

	virtual void accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor );

	CAddress getServiceAddress() const;

	std::vector< unsigned char > const & getPayload() const;

	void setMediumPtr( uintptr_t _mediumPtr );

	uintptr_t getMediumPtr() const;

	CPubKey getPublicKey() const;

	void setPublicKey( CPubKey const & _pubKey );

	~CConnectNodeAction(){};
private:
	static int const ms_randomPayloadLenght = 32;

	std::vector< unsigned char > m_payload;

	bool const m_passive;

	CAddress m_addrConnect;

	CPubKey m_key;

	uintptr_t m_mediumPtr;
};


}

#endif // CONNECT_NODE_ACTION_H
