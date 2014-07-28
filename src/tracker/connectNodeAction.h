// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECT_NODE_ACTION_H
#define CONNECT_NODE_ACTION_H

#include "common/action.h"
#include "configureTrackerActionHandler.h"
#include <boost/statechart/state_machine.hpp>

#include "protocol.h"

namespace tracker
{

struct CConnectNodeActionUninitiated;

class CConnectNodeAction : public common::CAction< TrackerResponses >, public  boost::statechart::state_machine< CConnectNodeAction, CConnectNodeActionUninitiated >, public common::CCommunicationAction
{
public:
	CConnectNodeAction( std::string const & _nodeAddress );

	CConnectNodeAction( CAddress const & _addrConnect );

	CConnectNodeAction( uint256 const & _actionKey, std::vector< unsigned char > const & _payload, unsigned int _mediumKind );

	virtual common::CRequest< TrackerResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< TrackerResponses > & _visitor );

	void setRequest( common::CRequest< TrackerResponses >* _request );

	std::string getAddress() const;

	CAddress getServiceAddress() const;

	std::vector< unsigned char > getPayload() const;

	void setMediumKind( unsigned int _mediumKind );
// not safe
	unsigned int getMediumKind() const;

	~CConnectNodeAction(){};
private:
	common::CRequest< TrackerResponses >* m_request;

	std::string const m_nodeAddress;

	static int const ms_randomPayloadLenght = 32;

	std::vector< unsigned char > m_payload;

	unsigned int m_mediumKind;

	bool const m_passive;

	CAddress m_addrConnect;

};


}

#endif // CONNECT_NODE_ACTION_H
