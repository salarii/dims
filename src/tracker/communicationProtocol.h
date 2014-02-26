#ifndef COMMUNICATION_PROTOCOL_H
#define COMMUNICATION_PROTOCOL_H

class CommunicationProtocol
{
public:

private:

};

struct CPayloadKind
{
	enum Enum
	{
		Transactions,
		InfoRequest
	};
};


struct CHeader
{
	public:
	void setHash( uint256 const & _hash ) const;
	void setTime( int64_t const _time ) const;

	
	private:
	CPayloadKind::Enum m_payloadKind;
	uint256 m_hash;
	int64_t m_time;
	uint256 m_prevKey;
};


class CMessage
{
public:
	CMessage( void const * );
	CMessage( CMessage const & _message, uint256 const & _prevKey );
	template<typename S>
	void Serialize( S &s, uint256 _privateKey) const;

	~CMessage();
private:
	CHeader m_header;
	unsigned m_payloadSize;
	void const * m_payload;
};

CMessage::CMessage( void const * _payload, unsigned _size )
{
	m_header.setHash( Hash( _payload,  _size )  );
	m_header.setTime( GetTime() );
	m_header.m_prevKey = 0;
}

CMessage::CMessage( CMessage const & _message, uint256 const & _prevKey )
{
	char const *prevMessage = _message;

	m_header.setHash( Hash( prevMessage, prevMessage + sizeof( CMessage ) ) );
	m_header.setTime( GetTime() );
	
	m_header.m_prevKey = _prevKey;
}

CMessage::~CMessage()
{
	if ( m_payload )
		delete m_payload;
}

template<typename S>
void Serialize(S &s, Encryptor) const
{
	<< Encryptor ( m_header );
	<< m_payload;
}


void
unwindMessage( _message, Decriptor )
{
//check  time 
		int64_t currentTime;


	Decriptor << _message 

	m_payload

// reset  decryptor
	unwindMessage( _message, Decriptor )


	CMessage 
	
	_message

	throw
}

#endif // COMMUNICATION_PROTOCOL_H