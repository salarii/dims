// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_RESPOND_H
#define REQUEST_RESPOND_H

#include <boost/variant.hpp>

#include "uint256.h"
#include "common/transactionStatus.h"
#include "errorRespond.h"

namespace node
{

struct CTransactionStatus
{
	common::TransactionsStatus::Enum m_status;
	uint256 m_token;
};

struct CAccountBalance
{

};

struct CTrackerStats
{
    CTrackerStats( std::string _publicKey = "", unsigned int  _reputation = 0, float _price = 0.0, std::string _ip = "", unsigned int _port = -1 )
        :m_publicKey( _publicKey ), m_reputation( _reputation ), m_price( _price ), m_ip( _ip ), m_port( _port ){};
	std::string m_publicKey;
	unsigned int  m_reputation;
	float m_price;
	std::string m_ip;
    unsigned int m_port;
};

struct CMonitorInfo
{
	std::vector< std::string > m_info;
};

struct CPending
{
	CPending( uint256 const & _token ):m_token(_token){};
	uint256 m_token;
};

struct CSystemError
{
    CSystemError( ErrorType::Enum _errorType ):m_errorType(_errorType){};
    ErrorType::Enum m_errorType;
};

typedef boost::variant< CTransactionStatus, CAccountBalance, CTrackerStats, CMonitorInfo, CPending, CSystemError > RequestRespond;

}

#endif
