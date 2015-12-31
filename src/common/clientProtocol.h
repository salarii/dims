// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "serialize.h"
#include "core.h"
#include "net.h"
#include "coins.h"

#include <boost/statechart/event.hpp>

namespace common
{

struct CClientPayloadKind
{
enum Enum
{
	Transactions,
};
};

struct CClientBalanceAsk
{
	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_address);
	)

	CClientBalanceAsk(){}

	CClientBalanceAsk(std::string const & _address):m_address(_address){}

	std::string m_address;
};

struct CClientTransactionStatusAsk
{
	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_hash);
	)

	CClientTransactionStatusAsk(){}

	CClientTransactionStatusAsk(uint256 const & _hash):m_hash(_hash){}

	uint256 m_hash;
};



struct CClientTransactionSend
{
	IMPLEMENT_SERIALIZE
	(
			READWRITE(m_transaction);
	)

	CClientTransactionSend(){}

	CClientTransactionSend(CTransaction const & _transaction):m_transaction(_transaction){}

	CTransaction m_transaction;
};

struct CClientHeader
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_payloadKind);
		READWRITE(m_id);
	)

	CClientHeader(){}

	CClientHeader( int _payloadKind, uint256 const & _id )
		: m_payloadKind( _payloadKind )
		, m_id( _id )
	{}

	int m_payloadKind;
	uint256 m_id;
};

struct CClientMessage
{
public:
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_header);
		READWRITE(m_payload);
	)

	CClientMessage(){}

	CClientMessage( int _messageKind, std::vector< unsigned char > const & _payload, uint256 const & _id )
		: m_header( _messageKind, _id )
		, m_payload(_payload)
	{
	}

	~CClientMessage(){};

	CClientHeader m_header;
	std::vector< unsigned char > m_payload;
};

struct CClientMessageResponse : boost::statechart::event< CClientMessageResponse >
{
	CClientMessageResponse( CClientMessage const & _clientMessage, uintptr_t _nodePtr, std::string const & _ip )
		: m_clientMessage(_clientMessage)
		, m_nodePtr(_nodePtr)
		, m_ip(_ip)
	{}

	CClientMessage m_clientMessage;

	uintptr_t m_nodePtr;

	std::string m_ip;
};

template < class T >
void
convertClientPayload( CClientMessage const & _message,T & _outMessage )
{
	CBufferAsStream stream( (char*)&_message.m_payload.front(), _message.m_payload.size(), SER_NETWORK, PROTOCOL_VERSION );
	stream >> _outMessage;
}

}

#endif // CLIENT_PROTOCOL_H
