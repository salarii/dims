// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COMMUNICATION_PROTOCOL_H
#define COMMUNICATION_PROTOCOL_H

#include <boost/variant.hpp>

#include "serialize.h"
#include "core.h"
#include "net.h"
#include "coins.h"

#include "common/struct.h"
#include "common/segmentFileStorage.h"
#include "common/support.h"

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
		RegistrationTerms,
		Result,
		AdmitProof,
		ValidRegistration,
		AdmitAsk,
		Ping,
		Pong,
		Balance,
		ClientTransaction,
		ClientStatusTransaction,
		TrackerInfo,
		SynchronizationAsk,
		SynchronizationBitcoinHeader,
		EnterNetworkAsk,
		EnterNetworkCondition,
		RankingInfo,
		FullRankingInfo,
		ExtendRegistration
	};
};

struct CInfoKind
{
	enum Enum
	{
		  NetworkInfoAsk
		, RoleInfoAsk
		, IsRegistered
		, StorageInfoAsk
		, BitcoinHeaderAsk
		, BalanceAsk
		, RankingAsk
		, TrackerInfo  // good ???
		, EnterConditionAsk
		, RegistrationTermsAsk
		, ClientTrasactionStatus
		, IsAddmited
		, RankingFullInfo
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

	static bool unwindMessage(
			CMessage const & _message
			, CMessage & _originalMessage
			, int64_t const _time
			, CPubKey const & _pubKey );

	static bool unwindMessageAndParticipants(
			CMessage const & _message
			, CMessage & _originalMessage
			, int64_t const _time
			, CPubKey const &  _pubKey
			, std::vector< CPubKey > & _participants
			);
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

struct CRankingFullInfo
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_allyTrackers);
		READWRITE(m_allyMonitors);
		READWRITE(m_trackers);
		READWRITE(m_leadingKey);
	)

	CRankingFullInfo()
	{}

	CRankingFullInfo(
			std::set< CAllyTrackerData > const & _allyTrackers
			, std::set< CAllyMonitorData > const & _allyMonitors
			, std::set< CTrackerData > const & _trackers
			, int64_t _time
			, uint256 const & _leadingKey )
		: m_allyTrackers( _allyTrackers )
		, m_allyMonitors( _allyMonitors )
		, m_trackers( _trackers )
		, m_leadingKey(_leadingKey)
	{}

	std::set< CAllyTrackerData > m_allyTrackers;
	std::set< CAllyMonitorData > m_allyMonitors;
	std::set< CTrackerData > m_trackers;
	int64_t m_time;
	uint256 m_leadingKey;
};

struct CRankingInfo
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_trackers);
		READWRITE(m_time);
	)

	CRankingInfo(){};

	CRankingInfo(
			std::set< CTrackerData > const & _trackers
			, int64_t _time)
		: m_trackers( _trackers )
		, m_time(_time)
	{}

	std::set< CTrackerData > m_trackers;
	int64_t m_time;
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

	CNetworkRole(){}

	CNetworkRole( int _role ){ m_role = _role; }

	int m_role;
};

struct CSynchronizationAsk
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_dummy);
	)

	int m_dummy;
};

struct CBitcoinHeader
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_bitcoinHeader);
	)

	CBlockHeader m_bitcoinHeader;
};

struct CTrackerInfo
{
		IMPLEMENT_SERIALIZE
		(
			READWRITE( m_key );
			READWRITE( m_price );
		)

		CTrackerInfo(){}

		CTrackerInfo( CPubKey const & _key, uint64_t _price ): m_key( _key ), m_price( _price ){}

		CPubKey m_key;
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
	CSynchronizationSegmentHeader(){}

	CSynchronizationSegmentHeader( CSegmentHeader * _segmentHeader, unsigned int _blockIndex ):m_segmentHeader( _segmentHeader ), m_blockIndex(_blockIndex){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(*m_segmentHeader);
		READWRITE(m_blockIndex);
	)

	CSegmentHeader * m_segmentHeader;
	unsigned int m_blockIndex;
};

struct CSynchronizationGet
{
	CSynchronizationGet(){}

	CSynchronizationGet( unsigned int _kind, unsigned int _number ):m_kind( _kind ), m_number( _number ){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_kind);
		READWRITE(m_number);
	)

	unsigned int m_kind;
	unsigned int m_number;
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
struct CInfoResponseData
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_dummy);
	)
	int m_dummy;
};

struct CAdmitAsk
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_dummy);
	)
	int m_dummy;
};

struct CBalance
{
	CBalance( std::map< uint256, CCoins > const & _availableCoins, std::map< uint256, std::vector< CKeyID > > const & _transactionInputs )
		: m_availableCoins(_availableCoins)
		, m_transactionInputs(_transactionInputs)
	{}
	CBalance(){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_availableCoins);
		READWRITE(m_transactionInputs);
	)
	std::map< uint256, CCoins > m_availableCoins;
	std::map< uint256, std::vector< CKeyID > > m_transactionInputs;
};

struct CInfoRequestData
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_kind );
		READWRITE( m_payload );
	)
	CInfoRequestData(){};

	CInfoRequestData( int _kind, std::vector<unsigned char> const & _payload = std::vector<unsigned char>() ): m_kind( _kind ), m_payload( _payload ){};

	template < class T >
	CInfoRequestData( int _kind, T const & _payload ): m_kind( _kind )
	{
		createPayload( _payload, m_payload );
	};

	int m_kind;

	std::vector<unsigned char> m_payload;
};

struct CEnteranceTerms
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_price );
	)

	CEnteranceTerms()
		: m_price( 0 )
	{}

	CEnteranceTerms( unsigned int _price )
		: m_price( _price )
	{}

	unsigned int m_price;
};


struct CRegistrationTerms
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_price );
		READWRITE( m_period );
	)

	CRegistrationTerms():m_price( 0 ),m_period( 0 ){}

	CRegistrationTerms( unsigned int _price, uint64_t const & _period ):m_price( _price ), m_period( _period ){}

	unsigned int m_price;
	uint64_t m_period;
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
		READWRITE(m_headerSize);
		READWRITE(m_strageSize);
	)

	CSynchronizationInfo(){}

	CSynchronizationInfo( uint64_t _timeStamp, unsigned int _headerSize, unsigned int _strageSize):m_timeStamp( _timeStamp ), m_headerSize( _headerSize ), m_strageSize( _strageSize ){}

	uint64_t m_timeStamp;
	unsigned int m_headerSize;
	unsigned int m_strageSize;
};

struct CResult
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_result );
	)

	CResult(){}

	CResult( unsigned int _result ){ m_result = _result; }

	unsigned int m_result;
};

struct CAdmitProof
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_proofTransactionHash );
	)
	CAdmitProof(){}

	CAdmitProof( uint256 const & _proofTransactionHash )
		:m_proofTransactionHash( _proofTransactionHash )
	{}

	uint256 m_proofTransactionHash;
};

struct CClientTransaction
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_transaction );
	)

	CClientTransaction(){}

	CClientTransaction( CTransaction const & _transaction ): m_transaction(_transaction){}

	CTransaction m_transaction;
};

struct CClientTransactionStatus
{
	IMPLEMENT_SERIALIZE
	(
		READWRITE( m_status );
	)

	CClientTransactionStatus(){}

	CClientTransactionStatus( int _status ):m_status(_status){}

	unsigned int m_status;
};

struct CMessage
{
public:
	CMessage();
	CMessage( CIdentifyMessage const & _identifyMessage, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CAck const & _ack, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CInfoRequestData const & _infoRequest, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CInfoResponseData const & _infoResponse, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CTransactionsBundleStatus const & _transactionsBundleStatus, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CTransactionBundle const & _bundle, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CRegistrationTerms const & _connectCondition, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CMessage const & _message, CPubKey const & _prevKey, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CAdmitAsk const & _admit, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CBalance const & _balance, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CClientTransaction const & _clientTransaction, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CClientTransactionStatus const & _clientTransactionStatus, uint256 const & _actionKey, uint256 const & _id );
	CMessage( CTrackerInfo const & _trackerInfo, uint256 const & _actionKey, uint256 const & _id );
	CMessage( int _messageKind, std::vector< unsigned char > const & _payload, uint256 const & _actionKey, uint256 const & _id );

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

}

#endif // COMMUNICATION_PROTOCOL_H
