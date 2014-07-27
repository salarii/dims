// Copyright (c) 2014 Dims dev-team
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
		Uninitiated,

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
	)

	CHeader( int _payloadKind, std::vector<unsigned char> const & _signedHash, int64_t _time, CPubKey const & _prevKey );
	int m_payloadKind;
	std::vector<unsigned char> m_signedHash;
	int64_t m_time;
	CPubKey m_prevKey;
};

struct CIdentifyMessage
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_payload);
		READWRITE(m_key);
		READWRITE(m_signed);
		READWRITE(m_actionKey);
	)

	std::vector<unsigned char> m_payload;
	CKeyID m_key;
	std::vector<unsigned char> m_signed;
	uint256 m_actionKey;
};

struct CNetworkRole
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_role);
		READWRITE(m_actionKey);
	)

	int m_role;
	uint256 m_actionKey;
};

struct CValidNodeInfo
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_key);
		READWRITE(m_address);
		READWRITE(m_role);
	)
	bool operator<( CValidNodeInfo const & _validNodeInfo ) const
	{
		return m_key < _validNodeInfo.m_key;
	}

	CKeyID m_key;
	CAddress m_address;
	int m_role;
};


struct CKnownNetworkInfo
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_networkInfo);
		READWRITE(m_actionKey);
	)

	std::vector< CValidNodeInfo > m_networkInfo;
	uint256 m_actionKey;
};

struct CSynchronizationInfo
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_timeStamp);
		READWRITE(m_actionKey);
	)

	uint64_t m_timeStamp;
	uint256 m_actionKey;
};

struct CMessage
{
public:
	CMessage();
	CMessage( CIdentifyMessage const & _identifyMessage );
	CMessage( CNetworkRole const & _networkRole );
	CMessage( CKnownNetworkInfo const & _knownNetworkInfo );
	CMessage( CSynchronizationInfo const & _synchronizationInfo );

	CMessage( std::vector< CTransaction > const & _bundle );
	CMessage( CMessage const & _message, CPubKey const & _prevKey, std::vector<unsigned char> const & _signedHash );
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

}

#endif // COMMUNICATION_PROTOCOL_H
