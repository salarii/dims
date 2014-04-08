// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "setResponseVisitor.h"
#include "sendTransactionAction.h"
#include "requestRespond.h"
#include "connectAction.h"
#include <boost/any.hpp>
#include <boost/optional.hpp>

namespace node
{
template < class T >
class CResponseVisitorBase : public boost::static_visitor< boost::optional< T > >
{
public:
	virtual boost::optional< T > operator()(CTransactionStatus & _transactionStatus ) const
	{
		return boost::optional< T >();
	}

	virtual boost::optional< T > operator()(CAccountBalance & _accountBalance ) const
	{
		return boost::optional< T >();
	}

	virtual boost::optional< T > operator()(CTrackerInfo & _accountBalance ) const
	{
		return boost::optional< T >();
	}

	virtual boost::optional< T > operator()(CMonitorInfo & _accountBalance ) const
	{
		return boost::optional< T >();
	}

	virtual boost::optional< T > operator()(CPending & _accountBalance ) const
	{
		return boost::optional< T >();
	}

};

class CGetTransactionStatus : public CResponseVisitorBase< self::TransactionsStatus::Enum >
{
public:
	boost::optional< self::TransactionsStatus::Enum > operator()(CTransactionStatus & _transactionStatus ) const
	{
		return _transactionStatus.m_status;
	}

};

class CGetToken : public CResponseVisitorBase< uint256 >
{
public:
	boost::optional< uint256 > operator()(CTransactionStatus & _transactionStatus ) const
	{
		return _transactionStatus.m_token;
	}
};

void 
CSetResponseVisitor::visit( CSendTransactionAction & _action )
{
	_action.setTransactionStatus(boost::apply_visitor( (CResponseVisitorBase< self::TransactionsStatus::Enum > const &)CGetTransactionStatus(), m_requestRespond ));
	_action.setTransactionToken(boost::apply_visitor( (CResponseVisitorBase< uint256 > const &)CGetToken(), m_requestRespond ));
}


void
CSetResponseVisitor::visit( CConnectAction & _action )
{
	_action.setInProgressToken(boost::apply_visitor( (CResponseVisitorBase< uint256 > const &)CGetToken(), m_requestRespond ));
}

void
CSetResponseVisitor::visit( CAction & _action )
{

}


}