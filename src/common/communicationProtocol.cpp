// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "communicationProtocol.h"
#include "authenticationProvider.h"
#include "util.h"
#include "version.h"
namespace common
{

bool
CommunicationProtocol::unwindMessage( CMessage const & _message, CMessage & _originalMessage, int64_t const _time, CPubKey const &  _pubKey )
{
	if ( _time < _message.m_header.m_time )
		return false;

	if ( _message.m_header.m_payloadKind != CPayloadKind::IntroductionReq )
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
CommunicationProtocol::signPayload( std::vector<unsigned char> const & _payload, std::vector<unsigned char> & _signedHash )
{
	uint256 hash = Hash(BEGIN(_payload), END(_payload));
	CAuthenticationProvider::getInstance()->sign( hash, _signedHash );
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

template < class T >
void
createPayload( T const & message, std::vector< unsigned char > & _payload )
{
	unsigned int size = ::GetSerializeSize( message, SER_NETWORK, PROTOCOL_VERSION );
	_payload.resize( size );
	CBufferAsStream stream( (char*)&_payload.front(), size, SER_NETWORK, PROTOCOL_VERSION );
	stream << message;
}

CMessage::CMessage( CIdentifyMessage const & _identifyMessage )
	: m_header( (int)CPayloadKind::IntroductionReq, std::vector<unsigned char>(), GetTime(), CPubKey() )
{
	createPayload( _identifyMessage, m_payload );
}

CMessage::CMessage( CNetworkRole const & _networkRole )
	: m_header( (int)CPayloadKind::IntroductionReq, std::vector<unsigned char>(), GetTime(), CPubKey() )
{
	createPayload( _networkRole, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CKnownNetworkInfo const & _knownNetworkInfo )
	: m_header( (int)CPayloadKind::IntroductionReq, std::vector<unsigned char>(), GetTime(), CPubKey() )
{
	createPayload( _knownNetworkInfo, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}


CMessage::CMessage( CMessage const & _message, CPubKey const & _prevKey, std::vector<unsigned char> const & _signedHash )
	: m_header( _message.m_header.m_payloadKind, _signedHash, GetTime(), _prevKey )
{
/*	m_payload = ( void* )new CMessage(_message);

	m_header.m_signedHash = _signedHash;

	m_header.m_time = GetTime();*/
}

uint256
getRandNumber()
{
	int const ComponentNumber = 8;
	uint32_t number[ ComponentNumber ];

	for( unsigned int i = 0; i < ComponentNumber; ++i )
	{
		number[i] = insecure_rand();
	}

	return *reinterpret_cast< uint256* >( &number[0] );
}

CNetworkActionRegister * CNetworkActionRegister::ms_instance = NULL;

CNetworkActionRegister*
CNetworkActionRegister::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CNetworkActionRegister();
	};
	return ms_instance;
}

bool
CNetworkActionRegister::isServicedByAction( uint256 const & _actionKey ) const
{
	std::set< uint256 >::const_iterator iterator = m_actionsInProgress.find( _actionKey );

	return iterator != m_actionsInProgress.end();
}

void
CNetworkActionRegister::unregisterServicedByAction( uint256 const & _actionKey )
{
	m_actionsInProgress.erase( _actionKey );
}


void
CNetworkActionRegister::registerServicedByAction( uint256 const & _actionKey )
{
	m_actionsInProgress.insert( _actionKey );
}

CCommunicationAction::CCommunicationAction()
{
	m_actionKey = getRandNumber();

	CNetworkActionRegister::getInstance()->registerServicedByAction( m_actionKey );
}

CCommunicationAction::CCommunicationAction( uint256 const & _actionKey )
	: m_actionKey( _actionKey )
{
	CNetworkActionRegister::getInstance()->registerServicedByAction( m_actionKey );
}

CCommunicationAction::~CCommunicationAction()
{
	CNetworkActionRegister::getInstance()->unregisterServicedByAction( m_actionKey );
}

uint256
CCommunicationAction::getActionKey() const
{
	return m_actionKey;
}

}
