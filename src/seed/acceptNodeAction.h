// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACCEPT_NODE_ACTION_H
#define ACCEPT_NODE_ACTION_H

#include "common/action.h"
#include "common/communicationProtocol.h"
#include "configureSeedActionHandler.h"
#include <boost/statechart/state_machine.hpp>
#include <boost/optional.hpp>

namespace seed
{

struct CUninitiated;

class CAcceptNodeAction : public common::CAction< SeedResponses >, public  boost::statechart::state_machine< CAcceptNodeAction, CUninitiated >, public common::CCommunicationAction
{
public:
	CAcceptNodeAction( uint256 const & _actionKey, std::vector< unsigned char > const & _payload, uintptr_t _mediumPtr );

	CAcceptNodeAction( CAddress const & _nodeAddress );

	virtual common::CRequest< SeedResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< SeedResponses > & _visitor );

	void setRequest( common::CRequest< SeedResponses >* _request );

	CAddress getAddress() const;

	std::vector< unsigned char > const & getPayload() const;

	void setMediumPtr( uintptr_t _mediumPtr );
// not safe
	uintptr_t getMediumPtr() const;

	bool getValid() const{ return m_valid; }

	void setValid( bool _valid ){ m_valid = _valid; }

	~CAcceptNodeAction(){};
private:
	common::CRequest< SeedResponses >* m_request;
	CAddress const m_nodeAddress;

	static int const ms_randomPayloadLenght = 32;

	std::vector< unsigned char > m_payload;

	uintptr_t m_mediumPtr;

	bool const m_passive;

	bool m_valid;
};


}

#endif // ACCEPT_NODE_ACTION_H
