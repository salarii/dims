// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "communicationProtocol.h"
#include "authenticationProvider.h"
#include "util.h"
#include "version.h"
namespace tracker
{

bool
CommunicationProtocol::unwindMessage( CMessage const & _message, CMessage & _originalMessage, int64_t const _time, CPubKey const &  _pubKey )
{
	if ( _time < _message.m_header.m_time )
		return false;

	if ( _message.m_header.m_payloadKind != CPayloadKind::Introduction )
	{
	uint256 messageHash = 	Hash( &_message.m_payload.front(), &_message.m_payload.back() );

	if ( !_pubKey.Verify(messageHash, _message.m_header.m_signedHash ) )
		return false;
	}

	if( _message.m_header.m_prevKey.IsValid() )
	{
		return unwindMessage(*(CMessage*)&_message.m_payload[0], _originalMessage, _message.m_header.m_time, _pubKey );
	}

	_originalMessage = _message;
	//_payload =*(Payload *)_message.m_payload;

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

CHeader::CHeader( int _payloadKind, std::vector<unsigned char> const & _signedHash, int64_t _time, CPubKey const & _prevKey )
	: m_payloadKind( (int)_payloadKind )
	, m_signedHash( _signedHash )
	, m_time( _time )
	, m_prevKey( _prevKey )
{
}

CMessage::CMessage()
	: m_header( (int)CPayloadKind::Uninitiated, std::vector<unsigned char>(), 0, CPubKey() )
{
}

CMessage::CMessage( std::vector< CTransaction > const & _bundle )
	: m_header( (int)CPayloadKind::Transactions, std::vector<unsigned char>(), GetTime(), CPubKey() )
{
	unsigned int size = ::GetSerializeSize( _bundle, SER_NETWORK, PROTOCOL_VERSION );
	m_payload.resize( size );
	CBufferAsStream stream( (char*)&m_payload.front(), size, SER_NETWORK, PROTOCOL_VERSION );
	stream << _bundle;

	uint256 hash = Hash( &m_payload.front(), &m_payload.back() );

	CAuthenticationProvider::getInstance()->sign( hash, m_header.m_signedHash );
}

CMessage::CMessage( CIdentifyMessage const & _identifyMessage )
	: m_header( (int)CPayloadKind::Introduction, std::vector<unsigned char>(), GetTime(), CPubKey() )
{
	unsigned int size = ::GetSerializeSize( _identifyMessage, SER_NETWORK, PROTOCOL_VERSION );
	m_payload.resize( size );
	CBufferAsStream stream( (char*)&m_payload.front(), size, SER_NETWORK, PROTOCOL_VERSION );
	stream << _identifyMessage;
}

CMessage::CMessage( CMessage const & _message, CPubKey const & _prevKey, std::vector<unsigned char> const & _signedHash )
	: m_header( _message.m_header.m_payloadKind, _signedHash, GetTime(), _prevKey )
{
/*	m_payload = ( void* )new CMessage(_message);

	m_header.m_signedHash = _signedHash;

	m_header.m_time = GetTime();*/
}


}
