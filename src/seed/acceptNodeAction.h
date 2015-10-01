// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACCEPT_NODE_ACTION_H
#define ACCEPT_NODE_ACTION_H

#include <boost/statechart/state_machine.hpp>
#include <boost/optional.hpp>

#include "common/action.h"
#include "common/communicationProtocol.h"

#include "seedDb.h"

namespace seed
{

void addResult( std::string const & _key, bool _value );

bool
getResult( std::string const & _key, bool & _value );

struct CUninitiated;

class CAcceptNodeAction : public common::CAction, public  boost::statechart::state_machine< CAcceptNodeAction, CUninitiated >
{
public:
	CAcceptNodeAction( uint256 const & _actionKey, uintptr_t _nodePtr, CServiceResult & _service );// last  is  because  of  ugliness

	CAcceptNodeAction( CServiceResult & _service );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	void setAddress( CAddress const & _address );

	CAddress getAddress() const;

	std::vector< unsigned char > const & getPayload() const;

	void setNodePtr( uintptr_t _nodePtr );
// not safe
	uintptr_t getNodePtr() const;

	bool getValid() const{ return m_service.fGood; }

	void setValid( bool _valid )
	{
		m_service.fGood = _valid;
	}

	CPubKey getPublicKey() const;

	void setPublicKey( CPubKey const & _pubKey );

	~CAcceptNodeAction(){};
private:
	CAddress m_nodeAddress;

	static int const ms_randomPayloadLenght = 32;

	std::vector< unsigned char > m_payload;

	uintptr_t m_nodePtr;

	bool const m_passive;

	bool m_valid;

	CPubKey m_key;

	CServiceResult & m_service;
};


}

#endif // ACCEPT_NODE_ACTION_H
