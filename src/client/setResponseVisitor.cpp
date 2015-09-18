// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/responses.h"
#include "common/events.h"
#include "common/responseVisitorInternal.h"

#include "sendTransactionAction.h"
#include "connectAction.h"
#include "sendBalanceInfoAction.h"
#include "payLocalApplicationAction.h"

#include "clientResponses.h"
#include "clientEvents.h"

#include <boost/any.hpp>
#include <boost/optional.hpp>

namespace common
{

template < class _Action >
class CGetMediumError : public CResponseVisitorBase< _Action, common::DimsResponsesList >
{
public:
	CGetMediumError( _Action * const _action ):CResponseVisitorBase< _Action, common::DimsResponsesList >( _action ){};

	void operator()(CMediumException & _systemError ) const
    {
    }
};
class CSetTransactionAction : public CResponseVisitorBase< client::CSendTransactionAction, common::DimsResponsesList >
{
public:
	CSetTransactionAction( client::CSendTransactionAction * const _action ):CResponseVisitorBase< client::CSendTransactionAction, common::DimsResponsesList >( _action ){};

};

class CSetBalanceInfoAction : public CResponseVisitorBase< client::CSendBalanceInfoAction, common::DimsResponsesList >
{
public:
	CSetBalanceInfoAction( client::CSendBalanceInfoAction * const _action ):CResponseVisitorBase< client::CSendBalanceInfoAction, common::DimsResponsesList >( _action ){};
	void operator()( common::CNoMedium & _noMedium ) const
	{
		LogPrintf("set response \"no medium\" to action: %p \n", this->m_action );
		this->m_action->process_event( _noMedium );
	}

};

class CSetConnectAction : public CResponseVisitorBase< client::CConnectAction, common::DimsResponsesList >
{
public:
	CSetConnectAction( client::CConnectAction * const _action ):CResponseVisitorBase< client::CConnectAction, common::DimsResponsesList >( _action ){};

	void operator()(common::CDnsInfo & _dnsInfo ) const
	{
		LogPrintf("set response \"dns info\" to action: %p \n", this->m_action );
		this->m_action->process_event( _dnsInfo );
	}

	void operator()( common::CNoMedium & _noMedium ) const
	{
		LogPrintf("set response \"no medium\" to action: %p \n", this->m_action );
		this->m_action->process_event( _noMedium );
	}

	void operator()(common::CMediumException & _systemError ) const
	{
				LogPrintf("set response \"system error\" to action: %p \n", this->m_action );
		this->m_action->process_event( common::CErrorEvent() );
	}

};

class CSetPayLocalApplicationAction : public CResponseVisitorBase< client::CPayLocalApplicationAction, common::DimsResponsesList >
{
public:
	CSetPayLocalApplicationAction( client::CPayLocalApplicationAction * const _action ):CResponseVisitorBase< client::CPayLocalApplicationAction, common::DimsResponsesList >( _action ){};

	void operator()(common::CTimeEvent & _timeEvent )
	{
		LogPrintf("set response \"time event\" to action: %p \n", this->m_action );
		this->m_action->process_event( _timeEvent );
	}

	void operator()( common::CNoMedium & _noMedium ) const
	{
		LogPrintf("set response \"no medium\" to action: %p \n", this->m_action );
		this->m_action->process_event( _noMedium );
	}

};

void 
CSetResponseVisitor::visit( client::CSendTransactionAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< client::CSendTransactionAction, common::DimsResponsesList > const &)CSetTransactionAction( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( client::CConnectAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< client::CConnectAction, common::DimsResponsesList > const &)CSetConnectAction( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( client::CSendBalanceInfoAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< client::CSendBalanceInfoAction, common::DimsResponsesList > const &)CSetBalanceInfoAction( &_action ), m_responses );
}

void
CSetResponseVisitor::visit( client::CPayLocalApplicationAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< client::CPayLocalApplicationAction, common::DimsResponsesList > const &)CSetPayLocalApplicationAction( &_action ), m_responses );
}

}
