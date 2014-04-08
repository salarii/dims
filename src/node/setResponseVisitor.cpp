// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "setResponseVisitor.h"
#include "sendTransactionAction.h"
#include "requestRespond.h"
#include <boost/any.hpp>

namespace node
{
template < class T >
class CResponseVisitorBase : public boost::static_visitor<T>
{
public:
	virtual T operator()(CTransactionStatus & _transactionStatus ) const
	{
		throw std::exception();
	}

	virtual T operator()(CAccountBalance & _accountBalance ) const
	{
		throw std::exception();
	}

	virtual T operator()(CTrackerInfo & _accountBalance ) const
	{
		throw std::exception();
	}

	virtual T operator()(CMonitorInfo & _accountBalance ) const
	{
		throw std::exception();
	}

	virtual T operator()(CPending & _accountBalance ) const
	{
		throw std::exception();
	}

};

class CGetTransactionStatus : public CResponseVisitorBase< self::TransactionsStatus::Enum >
{
public:
	self::TransactionsStatus::Enum operator()(CTransactionStatus & _transactionStatus ) const
	{
		return _transactionStatus.m_status;
	}

};

class CGetToken : public CResponseVisitorBase< uint256 >
{
public:
	uint256 operator()(CTransactionStatus & _transactionStatus ) const
	{
		return _transactionStatus.m_token;
	}
};

void 
CSetResponseVisitor::visit( CSendTransactionAction & _sendTransaction )
{
	_sendTransaction.setTransactionStatus(boost::apply_visitor( (CResponseVisitorBase< self::TransactionsStatus::Enum > const &)CGetTransactionStatus(), m_requestRespond ));
	_sendTransaction.setTransactionToken(boost::apply_visitor( (CResponseVisitorBase< uint256 > const &)CGetToken(), m_requestRespond ));
}


void
CSetResponseVisitor::visit( CConnectAction & _action )
{

}

void
CSetResponseVisitor::visit( CAction & _action )
{

}


}