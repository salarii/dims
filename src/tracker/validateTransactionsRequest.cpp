// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "validateTransactionsRequest.h"
#include "common/medium.h"
#include "common/mediumKinds.h"

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
CValidateTransactionsRequest::getKind() const
{
	return common::CMediumKinds::Internal;
}

std::vector< CTransaction > const &
CValidateTransactionsRequest::getTransactions() const
{
	return m_transactions;
}

CTransactionsKnownRequest::CTransactionsKnownRequest()
{
}

void
CTransactionsKnownRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CTransactionsKnownRequest::getKind() const
{
	return common::CMediumKinds::Internal;
}

CTransactionsAckRequest::CTransactionsAckRequest()
{
}

void
CTransactionsAckRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CTransactionsAckRequest::getKind() const
{
	return common::CMediumKinds::Internal;
}

CTransactionsDoublespendRequest::CTransactionsDoublespendRequest()
{
}

void
CTransactionsDoublespendRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CTransactionsDoublespendRequest::getKind() const
{
	return common::CMediumKinds::Internal;
}

CTransactionsNotOkRequest::CTransactionsNotOkRequest()
{
}

void
CTransactionsNotOkRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CTransactionsNotOkRequest::getKind() const
{
	return common::CMediumKinds::Internal;
}

}
