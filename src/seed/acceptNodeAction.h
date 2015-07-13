// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACCEPT_NODE_ACTION_H
#define ACCEPT_NODE_ACTION_H

#include "common/action.h"
#include "common/communicationProtocol.h"
#include "common/types.h"

#include "configureSeedActionHandler.h"
#include <boost/statechart/state_machine.hpp>
#include <boost/optional.hpp>

namespace seed
{

extern boost::mutex mutex;
extern std::map< std::string, bool > m_result;

void addResult( std::string const & _key, bool _value );

bool
getResult( std::string const & _key, bool & _value );

struct CUninitiated;

class CAcceptNodeAction : public common::CAction< common::CSeedTypes >, public  boost::statechart::state_machine< CAcceptNodeAction, CUninitiated >
{
public:
	CAcceptNodeAction( uint256 const & _actionKey, uintptr_t _nodePtr );

	CAcceptNodeAction( CAddress const & _nodeAddress );

	virtual void accept( common::CSetResponseVisitor< common::CSeedTypes > & _visitor );

	void setAddress( CAddress const & _address );

	CAddress getAddress() const;

	std::vector< unsigned char > const & getPayload() const;

	void setNodePtr( uintptr_t _nodePtr );
// not safe
	uintptr_t getNodePtr() const;

	bool getValid() const{ return m_valid; }

	void setValid( bool _valid ){ addResult( m_nodeAddress.ToStringIP(), _valid ); }

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
};


}

#endif // ACCEPT_NODE_ACTION_H
