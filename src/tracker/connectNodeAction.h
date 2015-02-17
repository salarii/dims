// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECT_NODE_ACTION_H
#define CONNECT_NODE_ACTION_H

#include "common/action.h"
#include "common/filters.h"

#include "configureTrackerActionHandler.h"

#include <boost/statechart/state_machine.hpp>

#include "protocol.h"
/*
current communication protocol is ineffective
consider using ack  request after  every successful message  reception
*/


namespace tracker
{

struct CConnectNodeActionUninitiated;

class CConnectNodeAction : public common::CAction< TrackerResponses >, public  boost::statechart::state_machine< CConnectNodeAction, CConnectNodeActionUninitiated >, public common::CCommunicationAction
{
public:
	CConnectNodeAction( std::string const & _nodeAddress );

	CConnectNodeAction( CAddress const & _addrConnect );

	CConnectNodeAction( uint256 const & _actionKey, std::vector< unsigned char > const & _payload, uintptr_t _mediumPtr );

	virtual common::CRequest< TrackerResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< TrackerResponses > & _visitor );

	void setRequest( common::CRequest< TrackerResponses >* _request );

	common::CRequest< TrackerResponses > const * getRequest() const;

	std::string getAddress() const;

	void setServiceAddress( CAddress const & _addrConnect );

	CAddress getServiceAddress() const;

	std::vector< unsigned char > const & getPayload() const;

	void setMediumPtr( uintptr_t _mediumPtr );

	uintptr_t getMediumPtr() const;

	CPubKey getPublicKey() const;

	void setPublicKey( CPubKey const & _pubKey );

	~CConnectNodeAction(){};
private:
	common::CRequest< TrackerResponses >* m_request;

	std::string const m_nodeAddress;

	static int const ms_randomPayloadLenght = 32;

	std::vector< unsigned char > m_payload;

	bool const m_passive;

	CAddress m_addrConnect;

	CPubKey m_key;

	uintptr_t m_mediumPtr;
};


}

#endif // CONNECT_NODE_ACTION_H
