// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_MESSAGES_H
#define NODE_MESSAGES_H

namespace self
{

struct CClientMessageType
{
	enum Enum
	{
		  Transaction
		, TrackerInfoReq
		, MonitorInfoReq
		, TransactionInfoReq
	};
};

struct CServerMessageType
{
	enum Enum
	{
		  ReferenceToken
		, TransactionStatus
		, MonitorInfo
		, TrackerInfo
		, RequestSatatus
	};
};

struct CTransactionMessage
{
	CTransaction m_transaction;
};

struct CTrackerInfoReq
{
	uint256 m_trackerPubKey;
};

struct CMonitorInfoReq
{
	uint256 m_monitorPubKey;
};

struct CTransactionInfoReq
{
	uint256 m_transactionPubKey;
};


}

#endif