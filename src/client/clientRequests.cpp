// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "clientRequests.h"

#include "common/medium.h"
#include "common/support.h"
namespace client
{

CErrorForAppPaymentProcessing::CErrorForAppPaymentProcessing( dims::CAppError::Enum _error, common::CMediumFilter * _mediumFilter )
	: common::CRequest( _mediumFilter )
	, m_error( _error )
{

}

void
CErrorForAppPaymentProcessing::accept( common::CMedium * _medium ) const
{
	_medium->add( this );
}


CProofTransactionAndStatusRequest::CProofTransactionAndStatusRequest( CTransaction const & _trasaction, std::vector<unsigned char> const & _transactionStatusSignature, CPubKey const & _servicingTracker, common::CMonitorData const & _monitorData, CPubKey const & _servicingMonitor, common::CMediumFilter * _mediumFilter )
	: common::CRequest( _mediumFilter )
	, m_payApplicationData( _trasaction, _transactionStatusSignature, _servicingTracker, _monitorData, _servicingMonitor )
{
}

void
CProofTransactionAndStatusRequest::accept( common::CMedium * _medium ) const
{
	_medium->add( this );
}

}
