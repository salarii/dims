// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COMMUNICATION_PROTOCOL_H
#define COMMUNICATION_PROTOCOL_H

#include <boost/variant.hpp>

#include "serialize.h"
#include "core.h"
#include "net.h"
#include "coins.h"

#include "common/segmentFileStorage.h"

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
		Pong,
		Balance,
		ClientTransaction,
		ClientStatusTransaction,
		StorageInfo,
		TrackerInfo
	};
};

struct CInfoKind
{
	enum Enum
	{
		  NetworkInfoAsk
		, RoleInfoAsk
		, IsRegistered
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

struct CSynchronizationBlock;

struct CSynchronizationSegmentHeader;

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

struct CTransactionStorageInfo
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_storageSize );
		READWRITE( m_headerSize );
		READWRITE( m_time );
	)

	CTransactionStorageInfo(){}

	CTransactionStorageInfo( uint64_t _storageSize, uint64_t _headerSize, uint64_t _time ): m_storageSize( _storageSize ), m_headerSize( _headerSize ), m_time( _time ){}
	uint64_t m_storageSize;
	uint64_t m_headerSize;
	uint64_t m_time;
};

struct CTrackerInfo
{
		IMPLEMENT_SERIALIZE
		(
			READWRITE( m_price );
		)

		CTrackerInfo(){}

		CTrackerInfo( uint64_t _price ): m_price( _price ){}

		uint64_t m_price;
};

struct CTransactionBundle
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_transactions);
	)

	std::vector< CTransaction > m_transactions;
};

struct CSynchronizationBlock
{
	CSynchronizationBlock( CDiskBlock * _diskBlock, unsigned int _blockIndex ):m_diskBlock( _diskBlock ), m_blockIndex(_blockIndex){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(*m_diskBlock);
		READWRITE(m_blockIndex);
	)

	CDiskBlock * m_diskBlock;
	unsigned int m_blockIndex;
};

struct CSynchronizationSegmentHeader
{
	CSynchronizationSegmentHeader( CSegmentHeader * _segmentHeader, unsigned int _blockIndex ):m_segmentHeader( _segmentHeader ), m_blockIndex(_blockIndex){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(*m_segmentHeader);
		READWRITE(m_blockIndex);
	)

	CSegmentHeader * m_segmentHeader;
	unsigned int m_blockIndex;
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

struct CBalance
{
	CBalance( std::map< uint256, CCoins > const & _availableCoins );
	CBalance();

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_availableCoins);
	)
	std::map< uint256, CCoins > m_availableCoins;
};

struct CInfoRequestData
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_kind );
		READWRITE( m_payload );
	)
	CInfoRequestData(){};

	CInfoRequestData( int _kind ): m_kind( _kind ){};

	int m_kind;

	std::vector<unsigned char> m_payload;
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
		READWRITE( m_registrationTime );
		READWRITE( m_period );
	)

	CValidRegistration():m_key(), m_registrationTime( 0 ), m_period( 0 ){}

	CValidRegistration( CPubKey const & _key, uint64_t _registrationTime, uint64_t const & _period ):m_key( _key ), m_registrationTime( _registrationTime ),m_period( _period ){}

	CPubKey m_key;
	uint64_t m_registrationTime;
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
	)

	unsigned int m_result;
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

struct CClientTransaction
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_transaction );
	)
	CTransaction m_transaction;
};

struct CClientTransactionStatus
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_status );
	)
	unsigned int m_status;
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
	CMessage( CSynchronizationBlock const & _synchronizationBlock, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CSynchronizationSegmentHeader const & _synchronizationSegmentHeader, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CBalance const & _balance, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CClientTransaction const & _clientTransaction, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CClientTransactionStatus const & _clientTransactionStatus, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CTransactionStorageInfo const & _transactionStorageInfo, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CTrackerInfo const & _trackerInfo, uint256 const & _actionKey, uint256 const & _id );

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
