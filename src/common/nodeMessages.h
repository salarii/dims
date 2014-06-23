// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_MESSAGES_H
#define NODE_MESSAGES_H

#include "uint256.h"
#include "core.h"
#include <boost/variant.hpp>

namespace common
{

struct CMainRequestType
{
	enum Enum
	{
		  ContinueReq
		, Transaction
		, TransactionStatusReq
		, MonitorInfoReq
		, TrackerInfoReq
		, BalanceInfoReq
		, RequestSatatusReq
		, None
	};
};

struct CTransactionMessage
{
	CTransactionMessage( CTransaction const & _transaction ):m_transaction( _transaction ){};
	CTransaction m_transaction;
};

struct CTrackerStatsReq
{
	std::vector< int > m_requestedInfo;
};

struct CMonitorInfoReq
{
	uint256 m_monitorPubKey;
};

struct CTransactionInfoReq
{
	uint256 m_transactionPubKey;
};

struct CAddressBalanceReq
{
	CAddressBalanceReq( std::string const & _address ):m_address(_address){};
	std::string m_address;
};

typedef boost::variant< CTrackerStatsReq, CAddressBalanceReq, CTransactionMessage > NodeRequest;

}

#endif
