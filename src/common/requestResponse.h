// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_RESPONSE_H
#define REQUEST_RESPONSE_H

#include <boost/variant.hpp>

#include "uint256.h"
#include "common/transactionStatus.h"
#include "errorResponse.h"
#include "key.h"

#include <boost/statechart/event.hpp>

namespace common
{

struct CTransactionStatus
{
	common::TransactionsStatus::Enum m_status;
	uint256 m_token;
};

struct CAccountBalance
{

};

struct CNodeStatistic
{
	CNodeStatistic( CPubKey const & _key, std::string _ip, unsigned int _port ): m_key( _key ), m_ip( _ip ), m_port( _port ){}

	bool operator<( CNodeStatistic const & _nodeStats ) const
	{
		return m_key.GetID() < _nodeStats.m_key.GetID();
	}

	CPubKey m_key;
	std::string m_ip;
	unsigned int m_port;
};

struct CTrackerStats : boost::statechart::event< CTrackerStats >
{
	CTrackerStats( CPubKey _publicKey = CPubKey(), unsigned int  _reputation = 0, float _price = 0.0, std::string _ip = "")
		:m_publicKey( _publicKey ), m_reputation( _reputation ), m_price( _price ), m_ip( _ip ){}
	CPubKey m_publicKey;
	unsigned int  m_reputation;
	float m_price;
	std::string m_ip;
	uintptr_t m_nodeIndicator;

};

struct CUnidentifiedStats
{
	CUnidentifiedStats(	std::string const & _ip, unsigned int _port ):m_ip( _ip ), m_port( _port ){}

	bool operator<( CUnidentifiedStats const & _unidentifiedStats ) const
	{
		return m_ip < _unidentifiedStats.m_ip;
	}
	std::string m_ip;
	unsigned int m_port;
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
