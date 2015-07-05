// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "util.h"
#include "version.h"

#include "common/communicationProtocol.h"
#include "common/authenticationProvider.h"

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

CHeader::CHeader( int _payloadKind, std::vector<unsigned char> const & _signedHash, int64_t _time, CPubKey const & _prevKey, uint256 const & _actionKey, uint256 const & _id )
	: m_payloadKind( (int)_payloadKind )
	, m_signedHash( _signedHash )
	, m_time( _time )
	, m_prevKey( _prevKey )
	, m_actionKey( _actionKey )
	, m_id( _id )
{
}

CMessage::CMessage()
	: m_header( (int)CPayloadKind::Uninitiated, std::vector<unsigned char>(), 0, CPubKey(), uint256(), uint256() )
{
}

CMessage::CMessage( CTransactionBundle const & _bundle, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::Transactions, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _bundle, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CIdentifyMessage const & _identifyMessage, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::IntroductionReq, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _identifyMessage, m_payload );
}

CMessage::CMessage( CNetworkRole const & _networkRole, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::RoleInfo, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _networkRole, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CKnownNetworkInfo const & _knownNetworkInfo, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::NetworkInfo, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _knownNetworkInfo, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CSynchronizationInfo const & _synchronizationInfo, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::SynchronizationInfo, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _synchronizationInfo, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CGet const & _get, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::Get, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _get, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CEnd const & _end, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::End, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _end, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CInfoResponseData const & _infoResponse, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::InfoRes, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _infoResponse, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CInfoRequestData const & _infoRequest, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::InfoReq, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _infoRequest, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CAck const & _ack, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::Ack, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _ack, m_payload );
}

CMessage::CMessage( CTransactionsBundleStatus const & _transactionsBundleStatus, uint256 const & _actionKey, uint256 const & _id )
: m_header( (int)CPayloadKind::StatusTransactions, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _transactionsBundleStatus, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CMessage const & _message, CPubKey const & _prevKey, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)_message.m_header.m_payloadKind, std::vector<unsigned char>(), GetTime(), _prevKey, _actionKey, _id )
{
	createPayload( _message, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CRegistrationTerms const & _connectCondition, uint256 const & _actionKey, uint256 const & _id )
: m_header( (int)CPayloadKind::ConnectCondition, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _connectCondition, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CResult const & _result, uint256 const & _actionKey, uint256 const & _id )
: m_header( (int)CPayloadKind::Result, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _result, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CAdmitProof const & _admit, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::AdmitProof, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _admit, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CValidRegistration const & _valid, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::ValidRegistration, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _valid, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CAdmitAsk const & _admit, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::AdmitAsk, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _admit, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CPong const & _pong, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::Pong, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _pong, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CPing const & _ping, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::Ping, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _ping, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CSynchronizationBlock const & _synchronizationInfo, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)common::CPayloadKind::SynchronizationBlock, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	common::createPayload( _synchronizationInfo, m_payload );

	common::CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CSynchronizationSegmentHeader const & _synchronizationSegmentHeader, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)common::CPayloadKind::SynchronizationHeader, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	common::createPayload( _synchronizationSegmentHeader, m_payload );

	common::CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CBalance const & _balance, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)common::CPayloadKind::Balance, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	common::createPayload( _balance, m_payload );

	common::CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( CClientTransaction const & _clientTransaction, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)common::CPayloadKind::ClientTransaction, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	common::createPayload( _clientTransaction, m_payload );

	common::CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
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

CCommunicationRegisterObject::CCommunicationRegisterObject( uint256 const & _actionKey )
	: m_actionKey( _actionKey )
{
	CNetworkActionRegister::getInstance()->registerServicedByAction( m_actionKey );
}

CCommunicationRegisterObject::~CCommunicationRegisterObject()
{
	CNetworkActionRegister::getInstance()->unregisterServicedByAction( m_actionKey );
}

}
