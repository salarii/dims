// Copyright (c) 2014-2015 Dims dev-team
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
	{
		// this should be serviced in special way
		// clock of trackers may be not  synchronized
		// return false;
	}
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
	uint256 hash = Hash( &_payload.front(), &_payload.back() );
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

CHeader::CHeader( int _payloadKind, std::vector<unsigned char> const & _signedHash, int64_t _time, CPubKey const & _prevKey, uint256 const & _actionKey )
	: m_payloadKind( (int)_payloadKind )
	, m_signedHash( _signedHash )
	, m_time( _time )
	, m_prevKey( _prevKey )
	, m_actionKey( _actionKey )
{
}

CMessage::CMessage()
	: m_header( (int)CPayloadKind::Uninitiated, std::vector<unsigned char>(), 0, CPubKey(), uint256() )
{
}

CMessage::CMessage( std::vector< CTransaction > const & _bundle, uint256 const & _actionKey )
	: m_header( (int)CPayloadKind::Transactions, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _bundle, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CIdentifyMessage const & _identifyMessage, uint256 const & _actionKey )
	: m_header( (int)CPayloadKind::IntroductionReq, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _identifyMessage, m_payload );
}

CMessage::CMessage( CNetworkRole const & _networkRole, uint256 const & _actionKey )
	: m_header( (int)CPayloadKind::RoleInfo, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _networkRole, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CKnownNetworkInfo const & _knownNetworkInfo, uint256 const & _actionKey )
	: m_header( (int)CPayloadKind::NetworkInfo, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _knownNetworkInfo, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CSynchronizationInfo const & _synchronizationInfo, uint256 const & _actionKey )
	: m_header( (int)CPayloadKind::SynchronizationInfo, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _synchronizationInfo, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CGet const & _get, uint256 const & _actionKey )
	: m_header( (int)CPayloadKind::Get, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _get, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CEnd const & _end, uint256 const & _actionKey )
	: m_header( (int)CPayloadKind::End, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _end, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CInfoResponse const & _infoResponse, uint256 const & _actionKey )
	: m_header( (int)CPayloadKind::InfoRes, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _infoResponse, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CInfoRequest const & _infoRequest, uint256 const & _actionKey )
	: m_header( (int)CPayloadKind::InfoReq, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _infoRequest, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CAck const & _ack, uint256 const & _actionKey )
	: m_header( (int)CPayloadKind::Ack, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _ack, m_payload );
}

CMessage::CMessage( CTransactionsBundleStatus const & _transactionsBundleStatus, uint256 const & _actionKey )
: m_header( (int)CPayloadKind::StatusTransactions, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _transactionsBundleStatus, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CMessage const & _message, CPubKey const & _prevKey, uint256 const & _actionKey )
	: m_header( (int)_message.m_header.m_payloadKind, std::vector<unsigned char>(), GetTime(), _prevKey, _actionKey )
{
	createPayload( _message, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CConnectCondition const & _connectCondition, uint256 const & _actionKey )
: m_header( (int)CPayloadKind::ConnectCondition, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _connectCondition, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CResult const & _result, uint256 const & _actionKey )
: m_header( (int)CPayloadKind::Result, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey )
{
	createPayload( _result, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
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
