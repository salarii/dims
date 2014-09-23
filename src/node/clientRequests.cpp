// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clientRequests.h"

#include "common/medium.h"

namespace client
{

CInfoRequestContinue::CInfoRequestContinue( uint256 const & _token, common::CMediumFilter< NodeResponses > * _mediumFilter )
	: common::CRequest< NodeResponses >( _mediumFilter )
	, m_token( _token )
{
}

void
CInfoRequestContinue::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< NodeResponses > *
CInfoRequestContinue::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}


CTransactionStatusRequest::CTransactionStatusRequest( uint256 const & _transactionHash, common::CMediumFilter< NodeResponses > * _medium )
	: common::CRequest< NodeResponses >( _medium )
	, m_transactionHash( _transactionHash )
{
}

void
CTransactionStatusRequest::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< NodeResponses > *
CTransactionStatusRequest::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}

void
CTransactionSendRequest::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

CTransactionSendRequest::CTransactionSendRequest( CTransaction const & _transaction, common::CMediumFilter< NodeResponses > * _medium )
	: common::CRequest< NodeResponses >( _medium )
	, m_transaction( _transaction )
{
}

common::CMediumFilter< NodeResponses > *
CTransactionSendRequest::getMediumFilter() const
{
	return common::CRequest< NodeResponses >::m_mediumFilter;
}

CErrorForAppPaymentProcessing::CErrorForAppPaymentProcessing( dims::CAppError::Enum _error, common::CMediumFilter< NodeResponses > * _mediumFlter )
	: common::CRequest< NodeResponses >( _mediumFlter )
	, m_error( _error )
{

}

void
CErrorForAppPaymentProcessing::accept( common::CMedium< NodeResponses > * _medium ) const
{
	_medium->add( this );
}

}
