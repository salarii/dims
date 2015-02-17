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
		Result
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

struct CTransactionBundle
{
	CTransactionBundle( std::vector< CTransaction > const & _bundle );
	std::vector< CTransaction > m_bundle;

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_bundle);
	)
};

typedef boost::variant< CTransactionBundle > Payload;

class CMessage;

class CAuthenticationProvider;

class CommunicationProtocol
{
public:
	static bool signPayload( std::vector<unsigned char> const & _payload, std::vector<unsigned char> & _signedHash );

	bool createMessage( CMessage const & _inMessage, CMessage & _outMessage ) const;

	Payload retrieveData();

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
	)

	CHeader( int _payloadKind, std::vector<unsigned char> const & _signedHash, int64_t _time, CPubKey const & _prevKey, uint256 const & _actionKey );
	int m_payloadKind;
	std::vector<unsigned char> m_signedHash;
	int64_t m_time;
	CPubKey m_prevKey;
	uint256 m_actionKey;
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
		READWRITE(m_dummy);
	)
	int m_dummy;
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

struct CConnectCondition
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_price );
		READWRITE( m_period );
	)

	CConnectCondition():m_price( 0 ),m_period( 0 ){}
	CConnectCondition( unsigned int _price, uint256 const & _period ):m_price( _price ), m_period( _period ){}

	unsigned int m_price;
	uint256 m_period;
};


struct CKnownNetworkInfo
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_networkInfo);
	)

	std::vector< CValidNodeInfo > m_networkInfo;
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
	)
	unsigned int m_result;
};

struct CMessage
{
public:
	CMessage();
	CMessage( CIdentifyMessage const & _identifyMessage, uint256 const & _actionKey );
	CMessage( CNetworkRole const & _networkRole, uint256 const & _actionKey );
	CMessage( CKnownNetworkInfo const & _knownNetworkInfo, uint256 const & _actionKey );
	CMessage( CSynchronizationInfo const & _synchronizationInfo, uint256 const & _actionKey );
	CMessage( CAck const & _ack, uint256 const & _actionKey );
	CMessage( CGet const & _get, uint256 const & _actionKey );
	CMessage( CEnd const & _end, uint256 const & _actionKey );
	CMessage( CInfoRequestData const & _infoRequest, uint256 const & _actionKey );
	CMessage( CInfoResponseData const & _infoResponse, uint256 const & _actionKey );
	CMessage( CTransactionsBundleStatus const & _transactionsBundleStatus, uint256 const & _actionKey );
	CMessage( std::vector< CTransaction > const & _bundle, uint256 const & _actionKey );
	CMessage( CMessage const & _message, CPubKey const & _prevKey, uint256 const & _actionKey );
	CMessage( CConnectCondition const & _connectCondition, uint256 const & _actionKey );
	CMessage( CResult const & _result, uint256 const & _actionKey );

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

class CCommunicationAction
{
public:
	CCommunicationAction();

	CCommunicationAction( uint256 const & _actionKey );

	uint256 getActionKey() const;

	~CCommunicationAction();
protected:
	uint256 m_actionKey;
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
