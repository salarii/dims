// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "validateTransactionsRequest.h"
#include "common/medium.h"
namespace tracker
{

CValidateTransactionsRequest::CValidateTransactionsRequest( std::vector< CTransaction > const & _transactions )
	: m_transactions( _transactions )
{
}

void CValidateTransactionsRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CValidateTransactionsRequest::getKind()
{
	return 0;
}

std::vector< CTransaction > const &
CValidateTransactionsRequest::getTransactions() const
{
	return m_transactions;
}


}

