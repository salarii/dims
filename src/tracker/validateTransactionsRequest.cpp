// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "validateTransactionsRequest.h"
#include "common/medium.h"
#include "common/mediumKinds.h"
#include "trackerFilters.h"

namespace tracker
{

CValidateTransactionsRequest::CValidateTransactionsRequest( std::vector< CTransaction > const & _transactions )
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_transactions( _transactions )
{
}

void CValidateTransactionsRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

std::vector< CTransaction > const &
CValidateTransactionsRequest::getTransactions() const
{
	return m_transactions;
}

CTransactionsPropagationRequest::CTransactionsPropagationRequest( std::vector< CTransaction > const & _transactions, uint256 const & _actionKey, common::CMediumFilter< TrackerResponses > * _mediumFilter )
	: common::CRequest< TrackerResponses >( _mediumFilter )
	, m_transactions( _transactions )
	, m_actionKey( _actionKey )
{
}

void
CTransactionsPropagationRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

uint256
CTransactionsPropagationRequest::getActionKey() const
{
	return m_actionKey;
}

std::vector< CTransaction > const &
CTransactionsPropagationRequest::getTransactions() const
{
	return m_transactions;
}

CTransactionsKnownRequest::CTransactionsKnownRequest()
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
{
}

void
CTransactionsKnownRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

CTransactionsAckRequest::CTransactionsAckRequest()
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
{
}

void
CTransactionsAckRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

CTransactionsDoublespendRequest::CTransactionsDoublespendRequest()
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
{
}

void
CTransactionsDoublespendRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

CTransactionsNotOkRequest::CTransactionsNotOkRequest()
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
{
}

void
CTransactionsNotOkRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

}
