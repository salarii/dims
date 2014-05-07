// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "sendTransactionAction.h"
#include "common/requestResponse.h"
#include "connectAction.h"
#include "sendBalanceInfoAction.h"
#include "common/responseVisitorInternal.h"
#include "configureNodeActionHadler.h"

#include <boost/any.hpp>
#include <boost/optional.hpp>

namespace common
{
// I belive  this  crap will look and work much better when I will convert all action into  state machines
// optionally I can always fix some  issues with partial specialisations ( this  will be  even uglier though )
template < class _Action >
class CGetTransactionStatus : public CResponseVisitorBase< _Action, node::NodeResponseList >
{
public:
	CGetTransactionStatus( _Action * const _action ):CResponseVisitorBase< _Action, node::NodeResponseList >( _action ){};

	void operator()(CTransactionStatus & _transactionStatus ) const
	{
		this->m_action->setTransactionStatus( _transactionStatus.m_status );
	}

};

template < class _Action >
class CGetToken : public CResponseVisitorBase< _Action, node::NodeResponseList >
{
public:
	CGetToken( _Action * const _action ):CResponseVisitorBase< _Action, node::NodeResponseList >( _action ){};

	void operator()(CTransactionStatus & _transactionStatus ) const
	{
		this->m_action->setInProgressToken( _transactionStatus.m_token );
	}

	void operator()(CPending & _peding ) const
	{
		this->m_action->setInProgressToken( _peding.m_token );
	}
};

template < class _Action >
class CGetTrackerInfo : public CResponseVisitorBase< _Action, node::NodeResponseList >
{
public:
	CGetTrackerInfo( _Action * const _action ):CResponseVisitorBase< _Action, node::NodeResponseList >( _action ){};

	void operator()(CTrackerStats & _transactionStatus ) const
	{
		this->m_action->setTrackerInfo( _transactionStatus );
	}
};

template < class _Action >
class CGetMediumError : public CResponseVisitorBase< _Action, node::NodeResponseList >
{
public:
	CGetMediumError( _Action * const _action ):CResponseVisitorBase< _Action, node::NodeResponseList >( _action ){};

	void operator()(CSystemError & _systemError ) const
    {
		this->m_action->setMediumError( _systemError.m_errorType );
    }
};

template < class _Action >
class CGetBalance : public CResponseVisitorBase< _Action, node::NodeResponseList >
{
public:
	CGetBalance( _Action * const _action ):CResponseVisitorBase< _Action, node::NodeResponseList >( _action ){};

	void operator()(CAvailableCoins & _availableCoins ) const
	{
		this->m_action->setBalance( _availableCoins.m_availableCoins );
	}
};

CSetResponseVisitor< node::NodeResponses >::CSetResponseVisitor( node::NodeResponses const & _requestRespond )
	:m_requestResponse( _requestRespond )
{
}

void 
CSetResponseVisitor< node::NodeResponses >::visit( node::CSendTransactionAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< node::CSendTransactionAction, node::NodeResponseList > const &)CGetTransactionStatus< node::CSendTransactionAction >( &_action ), m_requestResponse );
	boost::apply_visitor( (CResponseVisitorBase< node::CSendTransactionAction, node::NodeResponseList > const &)CGetToken< node::CSendTransactionAction >( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< node::NodeResponses >::visit( node::CConnectAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< node::CConnectAction, node::NodeResponseList > const &)CGetToken< node::CConnectAction >( &_action ), m_requestResponse );
	boost::apply_visitor( (CResponseVisitorBase< node::CConnectAction, node::NodeResponseList > const &)CGetTrackerInfo< node::CConnectAction >( &_action ), m_requestResponse );
	boost::apply_visitor( (CResponseVisitorBase< node::CConnectAction, node::NodeResponseList > const &)CGetMediumError< node::CConnectAction >( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< node::NodeResponses >::visit( node::CSendBalanceInfoAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< node::CSendBalanceInfoAction, node::NodeResponseList > const &)CGetBalance< node::CSendBalanceInfoAction >( &_action ), m_requestResponse );
	boost::apply_visitor( (CResponseVisitorBase< node::CSendBalanceInfoAction, node::NodeResponseList > const &)CGetToken< node::CSendBalanceInfoAction >( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< node::NodeResponses >::visit( CAction< node::NodeResponses > & _action )
{
}


}
