#include "communicationProtocol.h"

namespace self
{

struct CTransactionBundle
{
	CTransactionBundle( std::vector< CTransaction > & const _bundle );
	std::vector< CTransaction > m_bundle;
	
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_bundle);
	)
};

inline
CTransactionBundle::CTransactionBundle( std::vector< CTransaction > & const _bundle )
: m_bundle( _bundle )
{
}


CMessage::CMessage( std::vector< CTransaction > & const _bundle, CKey const & _key )
{
	m_header.m_payloadKind = Transactions;
	m_header.m_time = GetTime() ;
	m_payload = (void *)new CTransactionBundle( _bundle );
}

CMessage::CMessage( CMessage const & _message, uint256 const & _prevKey, std::vector<unsigned char> const & _signedHash )
{
	m_payload = ( void* )new CMessage(_message);

	m_header.m_signedHash = _signedHash;

	m_header.m_time = GetTime();

	m_header.m_prevKey = _prevKey;
}


bool
unwindMessage( CMessage const & _message, CPubKey const & _pubKey,  int64_t const _time , Payload & _payload)
{
	if ( _time < _message.m_header.m_time )
		return false;

	uint256 messageHash = Hash(BEGIN(_message), END(_message));
	
	if ( !_pubKey.Verify(messageHash, _message.m_header.m_signedHash ) )
		return false;

	if( _message.m_header.m_prevKey.IsValid() )
	{
		return unwindMessage((CMessage*)&_message->m_payload, _pubKey, _payload, _message.m_header.m_time );
	}

	_payload =*(Payload *)_message.m_payload;

	return true;
}


}