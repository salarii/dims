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
		InfoRequest,
		Introduction,
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

	static bool unwindMessage( CMessage const & _message, CPubKey const & _pubKey, Payload const & _payload, int64_t const _time );
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
/*
	unsigned int GetSerializeSize(int nType, int nVersion) const
	{
		unsigned size = 0;
		size+= ::GetSerializeSize(VARINT(m_payloadKind), nType, nVersion);
		size+= m_signedHash.GetSerializeSize(nType, nVersion);
		size+= ::GetSerializeSize(VARINT(m_time), nType, nVersion);
		size+= m_prevKey.GetSerializeSize(nType, nVersion);

		return size;
	}

	template<typename Stream>
	void Unserialize(Stream& s, int nType, int nVersion)
	{
		s >> VARINT(m_payloadKind);
		::Unserialize( s, m_signedHash, nType, nVersion );
		s >> VARINT( m_time );
		m_prevKey.Unserialize( s, nType, nVersion );
	}

	template<typename Stream>
	void Serialize(Stream &s, int nType, int nVersion)
	{
		s<<VARINT(m_payloadKind);
		::Serialize( s, m_signedHash, nType, nVersion );
		s<<VARINT( m_time );
		m_prevKey.Serialize( s, nType, nVersion );
	}
*/
	CHeader( int _payloadKind, std::vector<unsigned char> const & _signedHash, int64_t _time, CPubKey const & _prevKey );
	int m_payloadKind;
	std::vector<unsigned char> m_signedHash;
	int64_t m_time;
	CPubKey m_prevKey;
};

template< int size >
struct CIdentifyMessage
{
//Hash(&ip[0], &ip[16]);
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_payload);
		READWRITE(m_key);
		READWRITE(m_signed);
	)
	unsigned char m_payload[ size ];
	CKeyID m_key;
	std::vector<unsigned char> m_signed;
};

struct CMessage
{
public:
	CMessage();
	CMessage( std::vector< CTransaction > const & _bundle );
	CMessage( CMessage const & _message, CPubKey const & _prevKey, std::vector<unsigned char> const & _signedHash );
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_header);
	//	READWRITE(m_payload);
	//	READWRITE();
	)

	~CMessage(){};

	//Payload determinePayload();
	CHeader m_header;
	std::vector< unsigned char > m_payload;
};


}

#endif // COMMUNICATION_PROTOCOL_H
