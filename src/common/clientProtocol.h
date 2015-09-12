#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include "serialize.h"
#include "core.h"
#include "net.h"
#include "coins.h"

namespace common
{

struct CClientPayloadKind
{
enum Enum
{
	Transactions,
};
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
	CClientMessage(){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_header);
		READWRITE(m_payload);
	)

	~CClientMessage(){};

	CClientMessage( int _messageKind, std::vector< unsigned char > const & _payload, uint256 const & _id )
		: m_header( _messageKind, _id )
		, m_payload(_payload)
	{
	}

	CClientHeader m_header;
	std::vector< unsigned char > m_payload;
};

}

#endif // CLIENT_PROTOCOL_H
