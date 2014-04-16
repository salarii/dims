// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "sendBalanceInfoAction.h"
#include "setResponseVisitor.h"

namespace node
{

CSendBalanceInfoAction::CSendBalanceInfoAction( std::string const _pubKey )
	: m_pubKey( _pubKey )
{
}

void
CSendBalanceInfoAction::accept( CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

CRequest*
CSendBalanceInfoAction::execute()
{

}

void
CSendBalanceInfoAction::setBalance( boost::optional< std::string > const & _balance )
{
	m_balance = _balance;
}

void
CSendBalanceInfoAction::setInProgressToken( boost::optional< uint256 > const & _token )
{
	m_token = _token;
}

}
