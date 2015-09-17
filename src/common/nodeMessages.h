// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_MESSAGES_H
#define NODE_MESSAGES_H

#include "uint256.h"
#include "core.h"
#include <boost/variant.hpp>
#include "common/visitorConfigurationUtilities.h"

namespace common
{

struct CMainRequestType
{
	enum Enum
	{
		  Transaction
		, TransactionStatusReq
		, MonitorInfoReq
		, TrackerInfoReq
		, BalanceInfoReq
		, RequestSatatusReq
		, NetworkInfoReq
		, None
	};
};

struct CTransactionMessage
{
	CTransactionMessage( CTransaction const & _transaction ):m_transaction( _transaction ){};
	CTransaction m_transaction;
};

struct CTransactionStatusReq
{
	CTransactionStatusReq( uint256 const & _hash ):m_hash( _hash ){};
	uint256 m_hash;
};

struct CTrackerStatsReq
{
};

struct CMonitorInfoReq
{
	CMonitorInfoReq(): m_monitorPubKey( 0 ){};
	CMonitorInfoReq( uint256 _monitorPubKey ): m_monitorPubKey( _monitorPubKey ){};
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

struct CNetworkInfoReq
{
	CNetworkInfoReq(){}
};

typedef boost::mpl::list< CTrackerStatsReq, CMonitorInfoReq, CAddressBalanceReq, CTransactionMessage, CNetworkInfoReq, CTransactionStatusReq > NodeRequestsList;

typedef boost::make_variant_over< NodeRequestsList >::type NodeRequests;

class CClientRequestVisitorHandlerBase : public boost::static_visitor< void >
{
public:
	void operator()( VisitorParam( NodeRequestsList ,0 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,1 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,2 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,3 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,4 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,5 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,6 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,7 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,8 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,9 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,10 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,11 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,12 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,13 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,14 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,15 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,16 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,17 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,18 ) const & ) const {}
	void operator()( VisitorParam( NodeRequestsList ,19 ) const & ) const {}
};

}

#endif
