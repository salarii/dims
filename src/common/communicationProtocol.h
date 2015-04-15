// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COMMUNICATION_PROTOCOL_H
#define COMMUNICATION_PROTOCOL_H

#include <boost/variant.hpp>
#include "serialize.h"
#include "core.h"
#include "net.h"


namespace common
{

struct CPayloadKind
{
	enum Enum
	{
		Transactions,
		InfoReq,
		InfoRes,
		IntroductionReq,
		IntroductionRes,
		RoleInfo,
		NetworkInfo,
		SynchronizationInfo,
		SynchronizationGet,
		SynchronizationBlock,
		SynchronizationHeader,
		Uninitiated,
		Ack,
		Get,
		AckTransactions,
		StatusTransactions,
		Message,
		End,
		ConnectCondition,
		Result,
		AdmitProof,
		ValidRegistration,
		AdmitAsk,
		Ping,
		Pong
	};
};

struct CInfoKind
{
	enum Enum
	{
		  NetworkInfoAsk
		, RoleInfoAsk
	};
};

struct CRole
{
	enum Enum
	{
		Tracker,
		Seed,
		Monitor
	};
};

class CMessage;

class CAuthenticationProvider;

class CommunicationProtocol
{
public:
	static bool signPayload( std::vector<unsigned char> const & _payload, std::vector<unsigned char> & _signedHash );

	bool createMessage( CMessage const & _inMessage, CMessage & _outMessage ) const;

	static bool unwindMessage( CMessage const & _message, CMessage & _originalMessage, int64_t const _time, CPubKey const & _pubKey );
private:
	CAuthenticationProvider * m_authenticationProvider;
};

struct CHeader
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_payloadKind);
		READWRITE(m_signedHash);
		READWRITE(m_time);
		READWRITE(m_prevKey);
		READWRITE(m_actionKey);
		READWRITE(m_id);
	)

	CHeader( int _payloadKind, std::vector<unsigned char> const & _signedHash, int64_t _time, CPubKey const & _prevKey, uint256 const & _actionKey, uint256 const & _id );
	int m_payloadKind;
	std::vector<unsigned char> m_signedHash;
	int64_t m_time;
	CPubKey m_prevKey;
	uint256 m_actionKey;
	uint256 m_id;
};

struct CIdentifyMessage
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_payload);
		READWRITE(m_key);
		READWRITE(m_signed);
	)

	std::vector<unsigned char> m_payload;
	CPubKey m_key;
	std::vector<unsigned char> m_signed;
};

struct CTransactionsBundleStatus
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_status );
	)

	CTransactionsBundleStatus(){}
	CTransactionsBundleStatus( unsigned int _status ):m_status( _status ){}

	int m_status;
};

struct CNetworkRole
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_role);
	)

	int m_role;
};

struct CTransactionBundle
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_transactions);
	)

	std::vector< CTransaction > m_transactions;
};

struct CPing
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_dummy);
	)
	int m_dummy;
};

struct CPong
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_dummy);
	)
	int m_dummy;
};

struct CAck
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_dummy);
	)
	int m_dummy;
};

struct CEnd
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_dummy);
	)
	int m_dummy;
};

struct CInfoRequestData
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_kind );
	)
	CInfoRequestData(){};

	CInfoRequestData( int _kind ): m_kind( _kind ){};

	int m_kind;
};

struct CInfoResponseData
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_dummy);
	)
	int m_dummy;
};

struct CValidNodeInfo
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_key);
		READWRITE(m_address);
	)

	CValidNodeInfo()
	{
	}

	CValidNodeInfo( CPubKey _key, CAddress _address ):m_key( _key ), m_address( _address )
	{
	}

	bool operator<( CValidNodeInfo const & _validNodeInfo ) const
	{
		return m_key < _validNodeInfo.m_key;
	}

	CPubKey m_key;
	CAddress m_address;
};

struct CRegistrationTerms
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_price );
		READWRITE( m_period );
		READWRITE( m_id );
	)

	CRegistrationTerms():m_price( 0 ),m_period( 0 ){}

	CRegistrationTerms( unsigned int _price, uint64_t const & _period ):m_price( _price ), m_period( _period ){}

	unsigned int m_price;
	uint64_t m_period;
	uint256 m_id;
};

struct CValidRegistration
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_key );
		READWRITE( m_period );
	)

	CValidRegistration():m_key(),m_period( 0 ){}
	CValidRegistration( CPubKey const & _key, uint64_t const & _period ):m_key( _key ), m_period( _period ){}

	CPubKey m_key;
	uint64_t m_period;
};

struct CKnownNetworkInfo
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_trackersInfo);
		READWRITE(m_monitorsInfo);
	)

	std::set< CValidNodeInfo > m_trackersInfo;
	std::set< CValidNodeInfo > m_monitorsInfo;
};

struct CSynchronizationInfo
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_timeStamp);
	)

	uint64_t m_timeStamp;
};


struct CGet
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_type);
	)
	int m_type;
};

struct CResult
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_result );
		READWRITE( m_id );
	)

	unsigned int m_result;
	uint256 m_id;
};

struct CAdmitProof
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_proofTransactionHash );
	)
	uint256 m_proofTransactionHash;
};

struct CAdmitAsk
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_dummy);
	)
	int m_dummy;
};

struct CMessage
{
public:
	CMessage();
	CMessage( CIdentifyMessage const & _identifyMessage, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CNetworkRole const & _networkRole, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CKnownNetworkInfo const & _knownNetworkInfo, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CSynchronizationInfo const & _synchronizationInfo, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CAck const & _ack, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CGet const & _get, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CEnd const & _end, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CInfoRequestData const & _infoRequest, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CInfoResponseData const & _infoResponse, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CTransactionsBundleStatus const & _transactionsBundleStatus, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CTransactionBundle const & _bundle, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CRegistrationTerms const & _connectCondition, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CResult const & _result, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CAdmitProof const & _admit, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CMessage const & _message, CPubKey const & _prevKey, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CValidRegistration const & _valid, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CAdmitAsk const & _admit, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CPong const & _pong, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CPing const & _ping, uint256 const & _actionKey, uint256 const & _id );
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_header);
		READWRITE(m_payload);
	)

	~CMessage(){};

	//Payload determinePayload();
	CHeader m_header;
	std::vector< unsigned char > m_payload;
};


template < class T >
void
convertPayload( CMessage const & _message,T & _outMessage )
{
	CBufferAsStream stream( (char*)&_message.m_payload.front(), _message.m_payload.size(), SER_NETWORK, PROTOCOL_VERSION );
	stream >> _outMessage;
}

uint256
getRandNumber();

class CNetworkActionRegister
{
public:
	static CNetworkActionRegister* getInstance( );

	void registerServicedByAction( uint256 const & _actionKey );

	void unregisterServicedByAction( uint256 const & _actionKey );

	bool isServicedByAction( uint256 const & _actionKey ) const;
private:
	static CNetworkActionRegister * ms_instance;

	std::set< uint256 > m_actionsInProgress;
};
// a bit shitty /should be  virtual inheritance ??
class CCommunicationAction
{
public:
	CCommunicationAction( uint256 const & _actionKey );

	~CCommunicationAction();
private:
	uint256 const & m_actionKey;
};

template < class T >
void
createPayload( T const & message, std::vector< unsigned char > & _payload )
{
	unsigned int size = ::GetSerializeSize( message, SER_NETWORK, PROTOCOL_VERSION );
	_payload.resize( size );
	CBufferAsStream stream( (char*)&_payload.front(), size, SER_NETWORK, PROTOCOL_VERSION );
	stream << message;
}

}

#endif // COMMUNICATION_PROTOCOL_H
