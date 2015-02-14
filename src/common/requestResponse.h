// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_RESPONSE_H
#define REQUEST_RESPONSE_H

#include <boost/variant.hpp>

#include "uint256.h"
#include "common/transactionStatus.h"
#include "errorResponse.h"
#include "key.h"
#include "core.h"

#include <boost/statechart/event.hpp>

namespace common
{

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



}

#endif // REQUEST_RESPONSE_H
