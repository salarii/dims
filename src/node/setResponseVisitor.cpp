// Copyright (c) 2014 Dims dev-team
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
class CGetTransactionStatus : public CResponseVisitorBase< _Action, client::NodeResponseList >
{
public:
	CGetTransactionStatus( _Action * const _action ):CResponseVisitorBase< _Action, client::NodeResponseList >( _action ){};

	void operator()(CTransactionStatus & _transactionStatus ) const
	{
		this->m_action->setTransactionStatus( _transactionStatus.m_status );
	}

};

template < class _Action >
class CGetToken : public CResponseVisitorBase< _Action, client::NodeResponseList >
{
public:
	CGetToken( _Action * const _action ):CResponseVisitorBase< _Action, client::NodeResponseList >( _action ){};

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
class CGetTrackerInfo : public CResponseVisitorBase< _Action, client::NodeResponseList >
{
public:
	CGetTrackerInfo( _Action * const _action ):CResponseVisitorBase< _Action, client::NodeResponseList >( _action ){};

	void operator()(CTrackerStats & _transactionStatus ) const
	{
		this->m_action->setTrackerInfo( _transactionStatus );
	}
};

template < class _Action >
class CGetMediumError : public CResponseVisitorBase< _Action, client::NodeResponseList >
{
public:
	CGetMediumError( _Action * const _action ):CResponseVisitorBase< _Action, client::NodeResponseList >( _action ){};

	void operator()(CMediumException & _systemError ) const
    {
		this->m_action->setMediumError( _systemError.m_error );
    }
};

template < class _Action >
class CGetBalance : public CResponseVisitorBase< _Action, client::NodeResponseList >
{
public:
	CGetBalance( _Action * const _action ):CResponseVisitorBase< _Action, client::NodeResponseList >( _action ){};

	void operator()(CAvailableCoins & _availableCoins ) const
	{
		this->m_action->setBalance( _availableCoins.m_availableCoins );
	}
};

CSetResponseVisitor< client::NodeResponses >::CSetResponseVisitor( client::NodeResponses const & _requestRespond )
	:m_requestResponse( _requestRespond )
{
}

void 
CSetResponseVisitor< client::NodeResponses >::visit( client::CSendTransactionAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< client::CSendTransactionAction, client::NodeResponseList > const &)CGetTransactionStatus< client::CSendTransactionAction >( &_action ), m_requestResponse );
	boost::apply_visitor( (CResponseVisitorBase< client::CSendTransactionAction, client::NodeResponseList > const &)CGetToken< client::CSendTransactionAction >( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< client::NodeResponses >::visit( client::CConnectAction & _action )
{
	boost::apply_visitor( (CResponseVisitorBase< client::CConnectAction, client::NodeResponseList > const &)CGetToken< client::CConnectAction >( &_action ), m_requestResponse );
	boost::apply_visitor( (CResponseVisitorBase< client::CConnectAction, client::NodeResponseList > const &)CGetTrackerInfo< client::CConnectAction >( &_action ), m_requestResponse );
	boost::apply_visitor( (CResponseVisitorBase< client::CConnectAction, client::NodeResponseList > const &)CGetMediumError< client::CConnectAction >( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< client::NodeResponses >::visit( client::CSendBalanceInfoAction & _action )
{
	static int i = 0;
	if ( i == 2 )
	{
		i++;
		i--;

	}
	i++;
	boost::apply_visitor( (CResponseVisitorBase< client::CSendBalanceInfoAction, client::NodeResponseList > const &)CGetBalance< client::CSendBalanceInfoAction >( &_action ), m_requestResponse );
	boost::apply_visitor( (CResponseVisitorBase< client::CSendBalanceInfoAction, client::NodeResponseList > const &)CGetToken< client::CSendBalanceInfoAction >( &_action ), m_requestResponse );
	boost::apply_visitor( (CResponseVisitorBase< client::CSendBalanceInfoAction, client::NodeResponseList > const &)CGetMediumError< client::CSendBalanceInfoAction >( &_action ), m_requestResponse );
}

void
CSetResponseVisitor< client::NodeResponses >::visit( CAction< client::NodeResponses > & _action )
{
}


}
