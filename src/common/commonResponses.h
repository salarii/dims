// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_RESPONSES_H
#define TRACKER_RESPONSES_H

#include "coins.h"
#include "key.h"
#include "core.h"
#include "net.h"

#include "common/nodeMessages.h"
#include "common/transactionStatus.h"
#include "common/errorResponse.h"
#include "common/communicationProtocol.h"

#include <boost/variant.hpp>
#include <boost/statechart/event.hpp>
#include <boost/variant.hpp>

#include <exception>

namespace common
{

struct CNoMedium : boost::statechart::event< CNoMedium >
{};

struct CAvailableCoins : boost::statechart::event< CAvailableCoins >
{
	CAvailableCoins( std::map< uint256, CCoins > const & _availableCoins, uint256 const & _hash );
	CAvailableCoins();

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_hash);
		READWRITE(m_availableCoins);
	)

	static CMainRequestType::Enum const  m_requestType;
	uint256 m_hash;
	std::map< uint256, CCoins > m_availableCoins;
};

struct CIdentificationResult : boost::statechart::event< CIdentificationResult >
{
	CIdentificationResult( std::vector<unsigned char> const & _payload, std::vector<unsigned char> const & _signed, CPubKey const & _key, CAddress const & _address = CAddress() ):m_payload( _payload ),m_signed( _signed ),m_key( _key ),m_address(_address){};
	std::vector<unsigned char> m_payload;
	std::vector<unsigned char> m_signed;
	CPubKey m_key;
	CAddress m_address;
};

struct CGetPrompt
{
	CGetPrompt( int _type ):m_type( _type ){}
	int m_type;
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

struct CNetworkInfoResult
{
	CNetworkInfoResult( CPubKey const & _sourceKey, std::set< CValidNodeInfo > const & _trackersInfo, std::set< CValidNodeInfo > const & _monitorsInfo ):m_trackersInfo(_trackersInfo),m_monitorsInfo(_monitorsInfo){}

	CNetworkInfoResult(){}

	CPubKey m_sourceKey;
	std::set< CValidNodeInfo > m_trackersInfo;
	std::set< CValidNodeInfo > m_monitorsInfo;
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


template < class _Stats >
struct CNodeSpecific : public _Stats
{
	CNodeSpecific( _Stats const & _stats, std::string const & _ip, uintptr_t _nodeIndicator ):_Stats( _stats ), m_ip( _ip ), m_nodeIndicator( _nodeIndicator ){}
	std::string m_ip;
	uintptr_t m_nodeIndicator;
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
	CPingPongResult( bool _isPing, uintptr_t _nodePtr ):m_isPing( _isPing ), m_nodePtr( _nodePtr ){}

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

struct CTransactionStatus : boost::statechart::event< CTransactionStatus >
{
	CTransactionStatus(	common::TransactionsStatus::Enum _status, uint256 const & _transactionHash, std::vector<unsigned char> const & _signature ):m_status( _status ), m_transactionHash( _transactionHash ), m_signature( _signature ){}
	common::TransactionsStatus::Enum m_status;
	uint256 m_transactionHash;
	std::vector<unsigned char> m_signature;
};

struct CAccountBalance
{

};

struct CUnidentifiedNodeInfo
{
	IMPLEMENT_SERIALIZE
	(
	READWRITE(m_ip);
	READWRITE(m_port);
	)
	CUnidentifiedNodeInfo(	std::string const & _ip, unsigned int _port ):m_ip( _ip ), m_port( _port ){}
	std::string m_ip;
	unsigned int m_port;

	bool operator<( CUnidentifiedNodeInfo const & _unidentifiedStats ) const
	{
		return m_ip < _unidentifiedStats.m_ip;
	}
};

struct CNodeInfo : public CUnidentifiedNodeInfo
{
	IMPLEMENT_SERIALIZE
	(
	READWRITE(*(CUnidentifiedNodeInfo*)this);
	READWRITE(m_key);
	READWRITE(m_role);
	)

	CNodeInfo( CPubKey const & _key = CPubKey(), std::string _ip = std::string(), unsigned int _port = 0, unsigned int _role = -1 ): CUnidentifiedNodeInfo( _ip, _port ), m_key( _key ), m_role( _role ){}
	CPubKey m_key;
	unsigned int m_role;
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

struct CPending : boost::statechart::event< CPending >
{
	CPending( uint256 const & _token, uintptr_t _networkPtr ):m_token( _token ),m_networkPtr( _networkPtr ){};
	uint256 m_token;
	uintptr_t m_networkPtr;
};

struct CSystemError
{
	CSystemError( ErrorType::Enum _errorType ):m_errorType(_errorType){};
	ErrorType::Enum m_errorType;
};

struct CMonitorData
{
	// most likely self public key, should goes here
	IMPLEMENT_SERIALIZE
	(
			READWRITE( m_trackers );
			READWRITE( m_monitors );
			READWRITE( m_signed );
	)

	CMonitorData(){};

	CMonitorData( std::vector< common::CNodeInfo > const & _trackers, std::vector< common::CNodeInfo > const & _monitors ):m_trackers( _trackers ), m_monitors( _monitors ){};

	std::vector< common::CNodeInfo > m_trackers;
	std::vector< common::CNodeInfo > m_monitors;
	std::vector<unsigned char> m_signed;
	// recognized  monitors and trackers
};

struct CPayApplicationData
{
	CPayApplicationData(
			CTransaction const & _trasaction = CTransaction()
			, std::vector<unsigned char> const & _transactionStatusSignature = std::vector<unsigned char>()
			, CPubKey const & _servicingTracker = CPubKey()
			, common::CMonitorData const & _monitorData = common::CMonitorData()
			, CPubKey const & _servicingMonitor = CPubKey()
			);

	IMPLEMENT_SERIALIZE
	(
	READWRITE(m_trasaction);
	READWRITE(m_transactionStatusSignature);
	READWRITE(m_servicingTracker);
	READWRITE(m_monitorData);
	READWRITE(m_servicingMonitor);
	)
	CTransaction m_trasaction;
	std::vector<unsigned char> m_transactionStatusSignature;
	CPubKey m_servicingTracker;
	common::CMonitorData m_monitorData;
	CPubKey m_servicingMonitor;
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

typedef boost::variant< CNetworkInfoResult > ScheduledResult;

}

#endif // TRACKER_RESPONSES_H
