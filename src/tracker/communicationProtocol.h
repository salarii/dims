// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COMMUNICATION_PROTOCOL_H
#define COMMUNICATION_PROTOCOL_H

#include <boost/variant.hpp>
#include "serialize.h"
#include "core.h"

namespace tracker
{

struct CPayloadKind
{
	enum Enum
	{
		Transactions,
		InfoReq,
		InfoRes,
		IntroductionReq,
		IntroductionRes,
		Uninitiated
	};
};

struct CTransactionBundle
{
	CTransactionBundle( std::vector< CTransaction > const & _bundle );
	std::vector< CTransaction > m_bundle;

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_bundle);
	)
};

typedef boost::variant< CTransactionBundle > Payload;

class CMessage;

class CAuthenticationProvider;

class CommunicationProtocol
{
public:
	bool createMessage( Payload const & _payload, CMessage & _message ) const;

	bool createMessage( CMessage const & _inMessage, CMessage & _outMessage ) const;

	Payload retrieveData();

	static bool unwindMessage( CMessage const & _message, CMessage & _originalMessage, int64_t const _time, CPubKey const & _pubKey );
private:
	CAuthenticationProvider * m_authenticationProvider;
};
/*
CommunicationProtocol::prepareForSend( std::vector )
{
	uint256 messageHash = Hash(BEGIN(_message), END(_message));
	m_authenticationProvider->sign( messageHash );


}
*/
struct CHeader
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_payloadKind);
		READWRITE(m_signedHash);
		READWRITE(m_time);
		READWRITE(m_prevKey);
	)

	CHeader( int _payloadKind, std::vector<unsigned char> const & _signedHash, int64_t _time, CPubKey const & _prevKey );
	int m_payloadKind;
	std::vector<unsigned char> m_signedHash;
	int64_t m_time;
	CPubKey m_prevKey;
};

struct CIdentifyMessage
{
//Hash(&ip[0], &ip[16]);
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_payload);
		READWRITE(m_key);
		READWRITE(m_signed);
	)

	std::vector<unsigned char> m_payload;
	CKeyID m_key;
	std::vector<unsigned char> m_signed;
};

struct CMessage
{
public:
	CMessage();
	CMessage( CIdentifyMessage const & _identifyMessage );
	CMessage( std::vector< CTransaction > const & _bundle );
	CMessage( CMessage const & _message, CPubKey const & _prevKey, std::vector<unsigned char> const & _signedHash );
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_header);
		READWRITE(m_payload);
	)

	~CMessage(){};

	//Payload determinePayload();
	CHeader m_header;
	std::vector< unsigned char > m_payload;
};


template < class T >
void
convertPayload( CMessage const & _message,T & _outMessage )
{
	CBufferAsStream stream( (char*)&_message.m_payload.front(), _message.m_payload.size(), SER_NETWORK, PROTOCOL_VERSION );
	stream >> _outMessage;
}


}

#endif // COMMUNICATION_PROTOCOL_H
