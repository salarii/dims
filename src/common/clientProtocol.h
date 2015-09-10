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
		READWRITE(m_signedHash);
		READWRITE(m_time);
		READWRITE(m_prevKey);
		READWRITE(m_actionKey);
		READWRITE(m_id);
	)

	CClientHeader( int _payloadKind, int64_t _time, uint256 const & _actionKey, uint256 const & _id );

	int m_payloadKind;
	int64_t m_time;
	uint256 m_actionKey;
	uint256 m_id;
};

struct CClientMessage
{
public:
	CClientMessage();

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_header);
		READWRITE(m_payload);
	)

	~CClientMessage(){};

	CHeader m_header;
	std::vector< unsigned char > m_payload;
};

}

#endif // CLIENT_PROTOCOL_H
