// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COMMUNICATION_PROTOCOL_H
#define COMMUNICATION_PROTOCOL_H

namespace tracker
{

struct CPayloadKind
{
	enum Enum
	{
		Transactions,
		InfoRequest,
		Introduction
	};
};

typedef boost::variant< CTransactionBundle > Payload;

class CommunicationProtocol
{
public:
	send( CMessage const & _message );

	Payload retrieveData();
private:
	void unwindMessage( CMessage const & _message, CPubKey const &  _pubKey, Payload & _payload );
private:
	CAuthenticationProvider * m_authenticationProvider;
};

CommunicationProtocol::prepareForSend( std::vector )
{
	uint256 messageHash = Hash(BEGIN(_message), END(_message));
	m_authenticationProvider->sign( messageHash );


}

template<typename S>
void Serialize(S &s, int nType, int nVersion) const;

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
	private:
	CPayloadKind::Enum m_payloadKind;
	std::vector<unsigned char> m_signedHash;
	int64_t m_time;
	CPubKey m_prevKey;
};

	

class CMessage
{
public:
	CMessage( std::vector< CTransaction > _bundle );
	CMessage( CMessage const & _message, uint256 const & _prevKey );
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_header);
		READWRITE(determinePayload());
		READWRITE();
	)

	~CMessage();
private:
	Payload determinePayload();
private:
	CHeader m_header;
	void* m_payload;
};


}

#endif // COMMUNICATION_PROTOCOL_H
