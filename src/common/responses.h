// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef RESPONSES_H
#define RESPONSES_H

#include "coins.h"
#include "key.h"
#include "core.h"
#include "net.h"
#include "main.h"

#include "common/nodeMessages.h"
#include "common/transactionStatus.h"
#include "common/errorResponse.h"
#include "common/communicationProtocol.h"
#include "common/clientProtocol.h"

#include <boost/variant.hpp>
#include <boost/statechart/event.hpp>
#include <boost/variant.hpp>

#include <exception>

namespace common
{

struct CNoMedium : boost::statechart::event< CNoMedium >
{};

struct CAvailableCoinsData : boost::statechart::event< CAvailableCoinsData >
{
	CAvailableCoinsData(
			std::map< uint256
			, CCoins > const & _availableCoins
			, std::map< uint256, std::vector< CKeyID > > const & m_transactionInputs
			, uint256 const & _hash );

	CAvailableCoinsData();

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_hash);
		READWRITE(m_availableCoins);
		READWRITE(m_transactionInputs);
	)

	static CMainRequestType::Enum const  m_requestType;
	uint256 m_hash;
	std::map< uint256, CCoins > m_availableCoins;
	std::map< uint256, std::vector< CKeyID > > m_transactionInputs;
};


struct CTrackerStatsData
{

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_reputation);
		READWRITE(m_price);
		READWRITE(m_ip);
		READWRITE(m_nodeIndicator);
	)

	CTrackerStatsData(){}

	CTrackerStatsData( unsigned int _reputation, float _price, std::string _ip, uintptr_t _nodeIndicator )
		: m_reputation( _reputation )
		, m_price( _price )
		, m_ip( _ip )
		, m_nodeIndicator( _nodeIndicator ){};

	unsigned int  m_reputation;
	unsigned int m_price;
	std::string m_ip;
	uintptr_t m_nodeIndicator;
};

struct CIdentificationResult : boost::statechart::event< CIdentificationResult >
{

	CIdentificationResult( std::vector<unsigned char> const & _payload, std::vector<unsigned char> const & _signed, CPubKey const & _key, CAddress const & _address, uint256 const & _id )
		: m_payload( _payload )
		, m_signed( _signed )
		, m_key( _key )
		, m_address(_address)
		, m_id( _id ){};

	std::vector<unsigned char> m_payload;
	std::vector<unsigned char> m_signed;
	CPubKey m_key;
	CAddress m_address;
	uint256 m_id;
};

struct CExceptionInfo;
struct CNodeExceptionInfo;

struct CInterpreter
{
	virtual void visit( CExceptionInfo const * _exception )
	{
	}

	virtual void visit( CNodeExceptionInfo const * _exception )
	{
	}
};

struct CExceptionInfo
{
	virtual void accept( CInterpreter * _interpreter ) const
	{
		_interpreter->visit( this );
	}
};

struct CNodeExceptionInfo : public CExceptionInfo
{
	virtual void accept( CInterpreter * _interpreter ) const
	{
		_interpreter->visit( this );
	}
};

struct CMediumException : public std::exception
{
public:
	CMediumException(){};
	CMediumException(ErrorType::Enum _error):m_error(_error){};
	ErrorType::Enum m_error;

	CExceptionInfo * m_exceptionInfo;
};

struct CRoleResult
{
	CRoleResult(int _role):m_role( _role ){}
	int m_role;
};

struct CInfoAsk : boost::statechart::event< CInfoAsk >
{
	CInfoAsk( uint256 const & _id, CInfoKind::Enum _infoKind ) : m_id( _id ), m_infoKind( _infoKind ){}
	uint256 m_id;
	CInfoKind::Enum m_infoKind;
};

struct CNetworkInfoResult : boost::statechart::event< CNetworkInfoResult >
{
	CNetworkInfoResult(
			CValidNodeInfo const & _nodeSelfInfo
			, common::CRole::Enum _role
			,std::set< CValidNodeInfo > const & _trackersInfo
			, std::set< CValidNodeInfo > const & _monitorsInfo
			, bool _valid = true
			)
		:m_nodeSelfInfo( _nodeSelfInfo )
		, m_role( _role )
		, m_trackersInfo(_trackersInfo)
		, m_monitorsInfo(_monitorsInfo)
		, m_valid( _valid )
	{}

	CNetworkInfoResult():m_valid( false ){}

	CValidNodeInfo m_nodeSelfInfo;
	common::CRole::Enum m_role;
	std::set< CValidNodeInfo > m_trackersInfo;
	std::set< CValidNodeInfo > m_monitorsInfo;
	bool m_valid;
};


struct CClientNetworkInfoResult
{

	CClientNetworkInfoResult( std::vector< CValidNodeInfo > const & _networkInfo, CPubKey const & _selfKey, int _selfRole  ):m_networkInfo( _networkInfo ),m_selfKey( _selfKey ), m_selfRole( _selfRole ){}

	CClientNetworkInfoResult( std::vector< CValidNodeInfo > const & _networkInfo, CPubKey const & _selfKey, int _selfRole , std::string _ip, uintptr_t _nodeIndicator ):m_networkInfo( _networkInfo ),m_selfKey( _selfKey ), m_selfRole( _selfRole ){}

	CClientNetworkInfoResult(){}

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_networkInfo);
		READWRITE(m_selfKey);
		READWRITE(m_selfRole);
	)

	std::vector< CValidNodeInfo > m_networkInfo;
	CPubKey m_selfKey;
	int m_selfRole;
};

struct CTransactionStatusResponse
{
	IMPLEMENT_SERIALIZE
	(
	READWRITE(m_status);
	READWRITE(m_transactionHash);
	READWRITE(m_signedHash);
	)
	CTransactionStatusResponse(){}
	CTransactionStatusResponse(	int _status, uint256 const & _transactionHash, std::vector<unsigned char> const & _signedHash = std::vector<unsigned char>() ):m_status( _status ), m_transactionHash( _transactionHash ), m_signedHash(_signedHash){}
	int m_status;
	uint256 m_transactionHash;
	std::vector<unsigned char> m_signedHash;
};

struct CTransactionAck
{
	IMPLEMENT_SERIALIZE
	(
	READWRITE(m_status);
	READWRITE(m_transaction);
	)
	CTransactionAck(){}
	CTransactionAck( int _status, CTransaction const & _transaction ):m_status(_status),m_transaction(_transaction){}

	int m_status;
	CTransaction m_transaction;
};

struct CTrackerSpecificStats
{
	IMPLEMENT_SERIALIZE
	(
	READWRITE(m_price);
	)
	CTrackerSpecificStats(){};
	CTrackerSpecificStats( float _price ):m_price( _price ){};

	unsigned int m_price;
};

struct CTransactionAckData
{
	IMPLEMENT_SERIALIZE
	(
	READWRITE(m_status);
	READWRITE(m_transactionSend);
	)

	CTransactionAckData(){}

	CTransactionAckData( int _status, CTransaction _transactionSend ): m_status( _status ), m_transactionSend( _transactionSend ){}

	int m_status;
	CTransaction m_transactionSend;
};

struct CMonitorStatsData : boost::statechart::event< CMonitorStatsData >
{
	IMPLEMENT_SERIALIZE
	(
	READWRITE(m_monitorData);
	READWRITE(m_ip);
	READWRITE(m_nodeIndicator);
	)

	CMonitorStatsData()
	{}

	CMonitorStatsData( common::CMonitorData const & _monitorData, std::string _ip, uintptr_t _nodeIndicator )
		: m_monitorData( _monitorData )
		, m_ip( _ip )
		, m_nodeIndicator( _nodeIndicator ){};

	common::CMonitorData m_monitorData;
	std::string m_ip;
	uintptr_t m_nodeIndicator;
};

struct CTransactionStatus
{
	IMPLEMENT_SERIALIZE
	(
	READWRITE(m_status);
	READWRITE(m_transactionHash);
	READWRITE(m_signature);
	)

	CTransactionStatus(){}

	CTransactionStatus(	int _status, uint256 const & _transactionHash, std::vector<unsigned char> const & _signature ):m_status( _status ), m_transactionHash( _transactionHash ), m_signature( _signature ){}

	int m_status;
	uint256 m_transactionHash;
	std::vector<unsigned char> m_signature;
};

class CSelfNode;

struct CConnectedNode
{
	CConnectedNode( common::CSelfNode * _node ):m_node( _node ){};
	common::CSelfNode * m_node;
};



struct CAckResult
{
	CAckResult(){};
	CAckResult( uintptr_t _nodePtr ):m_nodePtr( _nodePtr )
	{}

	uintptr_t m_nodePtr;
};

struct CPingPongResult : boost::statechart::event< CPingPongResult >
{
	CPingPongResult( bool _isPing, uintptr_t _nodePtr ): m_nodePtr( _nodePtr ), m_isPing( _isPing ){}

	uintptr_t m_nodePtr;

	bool m_isPing;
};

struct CMessageResult : boost::statechart::event< CMessageResult >
{
	CMessageResult( CMessage const & _message, uintptr_t _nodeIndicator, CPubKey const & _pubKey = CPubKey() ): m_message( _message ),m_nodeIndicator( _nodeIndicator ), m_pubKey( _pubKey ){}

	CMessage m_message;
	uintptr_t m_nodeIndicator;
	CPubKey m_pubKey;
};

struct CTimeEvent : boost::statechart::event< CTimeEvent >
{
};

struct CAccountBalance
{

};

struct CMonitorInfo : public CNodeInfo
{
	CMonitorInfo( CPubKey const & _key = CPubKey(), std::string _ip = std::string(), unsigned int _port = 0, unsigned int _role = -1, std::set< CPubKey > const & _trackersKeys = std::set< CPubKey >() )
		: CNodeInfo( _key, _ip, _port, _role )
		, m_trackersKeys( _trackersKeys )
	{
	}

	CMonitorInfo( CNodeInfo const & _nodeInfo, std::set< CPubKey > const & _trackersKeys )
		: CNodeInfo( _nodeInfo )
		, m_trackersKeys( _trackersKeys )
	{
	}

	std::set< CPubKey > m_trackersKeys;
};

// add max/min price
struct CTrackerStats : public CNodeInfo
{
	CTrackerStats( CPubKey const & _publicKey = CPubKey(), unsigned int  _reputation = 0, float _price = 0.0, std::string _ip = "", unsigned int _port = -1, unsigned int _role = -1 )
		: CNodeInfo( _publicKey, _ip, _port, _role ), m_reputation( _reputation ), m_price( _price ){}
	unsigned int  m_reputation;
	unsigned int m_price;
};

struct CSystemError
{
	CSystemError( ErrorType::Enum _errorType ):m_errorType(_errorType){};
	ErrorType::Enum m_errorType;
};

struct CSynchronizationResult : boost::statechart::event< CSynchronizationResult >
{
	unsigned int m_result;
};

struct CExecutedIndicator : boost::statechart::event< CExecutedIndicator >
{
	CExecutedIndicator( bool _correct ):m_correct(_correct){}
	bool m_correct;
};

inline
uint256
hashMonitorData( CMonitorData const & _monitorData )
{
	std::vector< unsigned char > monitorsInBytes;
	std::vector< unsigned char > trackersInBytes;

	common::createPayload( _monitorData.m_monitors, monitorsInBytes );
	common::createPayload( _monitorData.m_trackers, trackersInBytes );

	monitorsInBytes.insert( monitorsInBytes.end(), trackersInBytes.begin(), trackersInBytes.end() );

	return Hash( &monitorsInBytes.front(), &monitorsInBytes.back() );
}

typedef boost::variant< CNetworkInfoResult, CTransactionAck, CValidRegistration, CSynchronizationResult, CExecutedIndicator, CAvailableCoinsData, CResult > ScheduledResult;

struct CRequestedMerkles
{
	CRequestedMerkles( std::vector< CMerkleBlock > const & _merkles, std::map< uint256 ,std::vector< CTransaction > > const & _transactions, uintptr_t _nodePtr ):m_merkles( _merkles ),m_transactions( _transactions ), m_nodePtr( _nodePtr ){};

	std::vector< CMerkleBlock > m_merkles;
	std::map< uint256 ,std::vector< CTransaction > > m_transactions;
	uintptr_t m_nodePtr;
};

struct CDnsInfo : boost::statechart::event< CDnsInfo >
{
	CDnsInfo( std::vector<CAddress> const & _addresses ):m_addresses( _addresses ){}
	vector<CAddress> m_addresses;
};

struct CValidationResult
{
	CValidationResult( std::vector< unsigned int > const & _invalidTransactionIndexes ):m_invalidTransactionIndexes( _invalidTransactionIndexes ){};
	std::vector< unsigned int > m_invalidTransactionIndexes;
};

typedef boost::mpl::list<
common::ScheduledResult
, common::CMediumException
, common::CAvailableCoinsData
, CValidationResult
, common::CConnectedNode
, common::CIdentificationResult
, common::CRequestedMerkles
, common::CMessageResult
, common::CPingPongResult
, common::CAckResult
, common::CTimeEvent
, common::CNoMedium
, common::CClientMessageResponse
, CDnsInfo > DimsResponsesList; // if you reach 20 limit pack them like in ScheduledResult

typedef boost::make_variant_over< DimsResponsesList >::type DimsResponse;



}

#endif // RESPONSES_H
