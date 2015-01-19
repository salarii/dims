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

struct CUnidentifiedStats
{
	CUnidentifiedStats(	std::string const & _ip, unsigned int _port ):m_ip( _ip ), m_port( _port ){}
	std::string m_ip;
	unsigned int m_port;

	bool operator<( CUnidentifiedStats const & _unidentifiedStats ) const
	{
		return m_ip < _unidentifiedStats.m_ip;
	}
};

struct CNodeStats : public CUnidentifiedStats
{
	CNodeStats( CPubKey const & _key = CPubKey(), std::string _ip = std::string(), unsigned int _port = 0, unsigned int _role = -1 ): CUnidentifiedStats( _ip, _port ), m_key( _key ), m_role( _role ){}
	CPubKey m_key;
	unsigned int m_role;
};


// add max/min price
struct CTrackerStats : public CNodeStats
{
	CTrackerStats( CPubKey _publicKey = CPubKey(), unsigned int  _reputation = 0, float _price = 0.0, unsigned int _maxPrice = 0, unsigned int _minPrice = 0, std::string _ip = "", unsigned int _port = -1 )
		: CNodeStats( _publicKey, _ip, _port ), m_reputation( _reputation ), m_price( _price ), m_maxPrice( _maxPrice ), m_minPrice( _minPrice ){}
	unsigned int  m_reputation;
	float m_price;
	unsigned int m_maxPrice;
	unsigned int m_minPrice;
};

struct CMonitorInfo
{
	std::vector< std::string > m_info;
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
