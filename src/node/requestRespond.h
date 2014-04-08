// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_RESPOND_H
#define REQUEST_RESPOND_H

#include <boost/variant.hpp>

#include "uint256.h"

namespace node
{

struct CTransactionStatus
{
	self::TransactionsStatus::Enum m_status;
	uint256 m_token;
};

struct CAccountBalance
{

};

struct CTrackerInfo
{
	std::vector< std::string > m_info;
};

struct CMonitorInfo
{
	std::vector< std::string > m_info;
};

struct CPending
{
	uint256 m_token;
};

typedef boost::variant< CTransactionStatus, CAccountBalance, CTrackerInfo, CMonitorInfo, CPending > RequestRespond;

}

#endif