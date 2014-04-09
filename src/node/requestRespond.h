// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_RESPOND_H
#define REQUEST_RESPOND_H

#include <boost/variant.hpp>

#include "uint256.h"
#include "common/transactionStatus.h"

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

struct CTrackerInfo
{
	CTrackerInfo( std::string const & _ip = "", std::string const & _price = "", std::string const & _rating = "" )
		:m_ip( _ip ),m_price( _price ),m_rating( _rating ){};
	std::string m_ip;
	std::string m_price;
	std::string m_rating;
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