// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "util.h"
#include "version.h"

#include "common/support.h"
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
CommunicationProtocol::unwindMessageAndParticipants(
		CMessage const & _message
		, CMessage & _originalMessage
		, int64_t const _time
		, CPubKey const &  _pubKey
		, std::vector< CPubKey > & _participants
		)
{
	if ( _time < _message.m_header.m_time )
	{
		// this should be serviced in special way
		// clock of nodes may be not  synchronized
		// return false;
	}
	if ( _message.m_header.m_payloadKind != CPayloadKind::IntroductionReq )
	{
	uint256 messageHash = 	Hash( &_message.m_payload.front(), &_message.m_payload.back() );

	if ( !_pubKey.Verify(messageHash, _message.m_header.m_signedHash ) )
		return false;
	}
	_participants.push_back( _pubKey );

	if( _message.m_header.m_prevKey.IsValid() )
	{
		return unwindMessageAndParticipants(*(CMessage*)&_message.m_payload[0], _originalMessage, _message.m_header.m_time, _pubKey, _participants );
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

CMessage::CMessage( CIdentifyMessage const & _identifyMessage, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::IntroductionReq, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _identifyMessage, m_payload );
}

CMessage::CMessage( CAck const & _ack, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)CPayloadKind::Ack, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
{
	createPayload( _ack, m_payload );
}

CMessage::CMessage( CMessage const & _message, CPubKey const & _prevKey, uint256 const & _actionKey, uint256 const & _id )
	: m_header( (int)_message.m_header.m_payloadKind, std::vector<unsigned char>(), GetTime(), _prevKey, _actionKey, _id )
{
	createPayload( _message, m_payload );

	CommunicationProtocol::signPayload( m_payload, m_header.m_signedHash );
}

CMessage::CMessage( int _messageKind, std::vector< unsigned char > const & _payload, uint256 const & _actionKey, uint256 const & _id )
	: m_header( _messageKind, std::vector<unsigned char>(), GetTime(), CPubKey(), _actionKey, _id )
	, m_payload(_payload)
{
		common::CommunicationProtocol::signPayload( _payload, m_header.m_signedHash );
}

}
