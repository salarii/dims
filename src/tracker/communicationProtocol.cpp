#include "communicationProtocol.h"
#include "authenticationProvider.h"
#include "util.h"
#include "version.h"
namespace tracker
{

bool
CommunicationProtocol::unwindMessage( CMessage const & _message, CPubKey const &  _pubKey, Payload const & _payload, int64_t const _time )
{
/*	if ( _time < _message.m_header.m_time )
		return false;

	uint256 messageHash = Hash(BEGIN(*_message.m_payload), END(_message));

	if ( !_pubKey.Verify(messageHash, _message.m_header.m_signedHash ) )
		return false;

	if( _message.m_header.m_prevKey.IsValid() )
	{
		return unwindMessage(*(CMessage*)_message.m_payload, _pubKey, _payload, _message.m_header.m_time );
	}

	//_payload =*(Payload *)_message.m_payload;
*/
	return true;
}

bool
CommunicationProtocol::createMessage( Payload const & _payload, CMessage & _message ) const
{
	return true;
}


bool
CommunicationProtocol::createMessage( CMessage const & _inMessage, CMessage & _outMessage ) const
{
	return true;
}

inline
CTransactionBundle::CTransactionBundle( std::vector< CTransaction > const & _bundle )
: m_bundle( _bundle )
{
}

CHeader::CHeader( CPayloadKind::Enum _payloadKind, std::vector<unsigned char> const & _signedHash, int64_t _time, CPubKey const & _prevKey )
	: m_payloadKind( _payloadKind )
	, m_signedHash( _signedHash )
	, m_time( _time )
	, m_prevKey( _prevKey )
{
}

CMessage::CMessage( std::vector< CTransaction > const & _bundle )
	: m_header( CPayloadKind::Transactions, std::vector<unsigned char>(), GetTime(), CPubKey() )
{
	unsigned int size = ::GetSerializeSize( _bundle, SER_NETWORK, PROTOCOL_VERSION );
	m_payload.resize( size );
	CBufferAsStream stream( (char*)&m_payload.front(), size, SER_NETWORK, PROTOCOL_VERSION );
	stream << _bundle;

	uint256 hash = Hash( &m_payload.front(), &m_payload.back() );

	CAuthenticationProvider::getInstance()->sign( hash, m_header.m_signedHash );

}

CMessage::CMessage( CMessage const & _message, CPubKey const & _prevKey, std::vector<unsigned char> const & _signedHash )
	: m_header( _message.m_header.m_payloadKind, _signedHash, GetTime(), _prevKey )
{
/*	m_payload = ( void* )new CMessage(_message);

	m_header.m_signedHash = _signedHash;

	m_header.m_time = GetTime();*/
}


}
