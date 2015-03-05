// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/commonResponses.h"
#include "common/commonEvents.h"
#include "common/responseVisitorInternal.h"

#include "sendTransactionAction.h"
#include "connectAction.h"
#include "sendBalanceInfoAction.h"
#include "configureClientActionHadler.h"
#include "payLocalApplicationAction.h"

#include "clientResponses.h"
#include "clientEvents.h"

#include <boost/any.hpp>
#include <boost/optional.hpp>

namespace common
{
/*
	virtual void operator()( common::CTimeEvent & _param ) const
	{
		this->m_action->process_event( _param );
	}
*/
template < class _Action >
class CGetMediumError : public CResponseVisitorBase< _Action, client::ClientResponseList >
{
public:
	CGetMediumError( _Action * const _action ):CResponseVisitorBase< _Action, client::ClientResponseList >( _action ){};

	void operator()(CMediumException & _systemError ) const
    {
    }
};
class CSetTransactionAction : public CResponseVisitorBase< client::CSendTransactionAction, client::ClientResponseList >
{
public:
	CSetTransactionAction( client::CSendTransactionAction * const _action ):CResponseVisitorBase< client::CSendTransactionAction, client::ClientResponseList >( _action ){};

	void operator()( CPending & _peding ) const
	{
		this->m_action->process_event( _peding );
	}

	void operator()( common::CTransactionStatus & _transactionStats ) const
	{
		this->m_action->process_event( _transactionStats );
	}

	void operator()( common::CTransactionAck & _transactionAck ) const
	{
		this->m_action->process_event( client::CTransactionAckEvent( ( common::TransactionsStatus::Enum )_transactionAck.m_status, _transactionAck.m_transaction ) );
	}
};

class CSetBalanceInfoAction : public CResponseVisitorBase< client::CSendBalanceInfoAction, client::ClientResponseList >
{
public:
	CSetBalanceInfoAction( client::CSendBalanceInfoAction * const _action ):CResponseVisitorBase< client::CSendBalanceInfoAction, client::ClientResponseList >( _action ){};

	void operator()( CPending & _peding ) const
	{
		this->m_action->process_event( _peding );
	}

	void operator()( common::CNoMedium & _noMedium ) const
	{
		this->m_action->process_event( _noMedium );
	}

	void operator()( common::CAvailableCoins & _availableCoins ) const
	{
		this->m_action->process_event( client::CCoinsEvent( _availableCoins.m_availableCoins ) );
	}
};

class CSetConnectAction : public CResponseVisitorBase< client::CConnectAction, client::ClientResponseList >
{
public:
	CSetConnectAction( client::CConnectAction * const _action ):CResponseVisitorBase< client::CConnectAction, client::ClientResponseList >( _action ){};

	void operator()(client::CDnsInfo & _dnsInfo ) const
	{
		this->m_action->process_event( _dnsInfo );
	}

	void operator()( common::CNoMedium & _noMedium ) const
	{
		this->m_action->process_event( _noMedium );
	}

	void operator()( common::CNodeSpecific< CClientNetworkInfoResult > & _networkInfo ) const
	{
		this->m_action->process_event( CClientNetworkInfoEvent( _networkInfo.m_networkInfo, _networkInfo.m_selfKey, _networkInfo.m_selfRole, _networkInfo.m_ip, _networkInfo.m_nodeIndicator ) );
	}

	void operator()( common::CNodeSpecific< CTrackerSpecificStats > & _trackerStats ) const
	{
		this->m_action->process_event( common::CTrackerStatsEvent( 0, _trackerStats.m_price, _trackerStats.m_ip, _trackerStats.m_nodeIndicator ) );
	}

	void operator()( common::CNodeSpecific< CMonitorData > & _monitorData ) const
	{
		this->m_action->process_event( common::CMonitorStatsEvent( _monitorData, _monitorData.m_ip, _monitorData.m_nodeIndicator ) );
	}

	void operator()(common::CMediumException & _systemError ) const
	{
		this->m_action->process_event( common::CErrorEvent() );
	}

	void operator()(CPending & _peding ) const
	{
		this->m_action->process_event( _peding );
	}
};

class CSetPayLocalApplicationAction : public CResponseVisitorBase< client::CPayLocalApplicationAction, client::ClientResponseList >
{
public:
	CSetPayLocalApplicationAction( client::CPayLocalApplicationAction * const _action ):CResponseVisitorBase< client::CPayLocalApplicationAction, client::ClientResponseList >( _action ){};

	void operator()(CPending & _peding ) const
	{
		this->m_action->process_event( _peding );
	}

	void operator()(common::CTimeEvent & _timeEvent )
	{
		this->m_action->process_event( _timeEvent );
	}

	void operator()( common::CTransactionStatus & _transactionStats ) const
	{
		this->m_action->process_event( _transactionStats );
	}

	void operator()( common::CNodeSpecific< CMonitorData > & _monitorData ) const
	{
		this->m_action->process_event( common::CMonitorStatsEvent( _monitorData, _monitorData.m_ip, _monitorData.m_nodeIndicator ) );
	}

	void operator()( common::CNoMedium & _noMedium ) const
	{
		this->m_action->process_event( _noMedium );
	}

	void operator()( common::CTransactionAck & _transactionAck ) const
	{
		this->m_action->process_event( client::CTransactionAckEvent( ( common::TransactionsStatus::Enum )_transactionAck.m_status, _transactionAck.m_transaction ) );
	}

};

CSetResponseVisitor< client::ClientResponses >::CSetResponseVisitor( client::ClientResponses const & _requestRespond )
	:m_requestResponse( _requestRespond )
{
}

void 
CSetResponseVisitor< client::ClientResponses >::visit( client::CSendTransactionAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< client::CSendTransactionAction, client::ClientResponseList > const &)CSetTransactionAction( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< client::ClientResponses >::visit( client::CConnectAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< client::CConnectAction, client::ClientResponseList > const &)CSetConnectAction( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< client::ClientResponses >::visit( client::CSendBalanceInfoAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< client::CSendBalanceInfoAction, client::ClientResponseList > const &)CSetBalanceInfoAction( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< client::ClientResponses >::visit( client::CPayLocalApplicationAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< client::CPayLocalApplicationAction, client::ClientResponseList > const &)CSetPayLocalApplicationAction( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< client::ClientResponses >::visit( CAction< client::ClientResponses > & _action )
{
}

}
