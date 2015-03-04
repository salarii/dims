// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clientRequests.h"

#include "common/medium.h"

namespace client
{

CTransactionStatusRequest::CTransactionStatusRequest( uint256 const & _transactionHash, common::CMediumFilter< ClientResponses > * _medium )
	: common::CRequest< ClientResponses >( _medium )
	, m_transactionHash( _transactionHash )
{
}

void
CTransactionStatusRequest::accept( common::CMedium< ClientResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMediumFilter< ClientResponses > *
CTransactionStatusRequest::getMediumFilter() const
{
	return common::CRequest< ClientResponses >::m_mediumFilter;
}

void
CTransactionSendRequest::accept( common::CMedium< ClientResponses > * _medium ) const
{
	_medium->add( this );
}

CTransactionSendRequest::CTransactionSendRequest( CTransaction const & _transaction, common::CMediumFilter< ClientResponses > * _medium )
	: common::CRequest< ClientResponses >( _medium )
	, m_transaction( _transaction )
{
}

common::CMediumFilter< ClientResponses > *
CTransactionSendRequest::getMediumFilter() const
{
	return common::CRequest< ClientResponses >::m_mediumFilter;
}

CErrorForAppPaymentProcessing::CErrorForAppPaymentProcessing( dims::CAppError::Enum _error, common::CMediumFilter< ClientResponses > * _mediumFilter )
	: common::CRequest< ClientResponses >( _mediumFilter )
	, m_error( _error )
{

}

void
CErrorForAppPaymentProcessing::accept( common::CMedium< ClientResponses > * _medium ) const
{
	_medium->add( this );
}


CProofTransactionAndStatusRequest::CProofTransactionAndStatusRequest( CTransaction const & _trasaction, std::vector<unsigned char> const & _transactionStatusSignature, CPubKey const & _servicingTracker, common::CMonitorData const & _monitorData, CPubKey const & _servicingMonitor, common::CMediumFilter< ClientResponses > * _mediumFilter )
	: common::CRequest< ClientResponses >( _mediumFilter )
	, m_payApplicationData( _trasaction, _transactionStatusSignature, _servicingTracker, _monitorData, _servicingMonitor )
{
}

void
CProofTransactionAndStatusRequest::accept( common::CMedium< ClientResponses > * _medium ) const
{
	_medium->add( this );
}


}
