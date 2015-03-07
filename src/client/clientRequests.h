// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLIENT_REQUESTS_H
#define CLIENT_REQUESTS_H

#include "configureClientActionHadler.h"

#include "common/request.h"
#include "common/commonResponses.h"
#include "appLib/messageType.h"
namespace client
{

struct CTransactionStatusRequest : public common::CRequest< common::CClientTypes >
{
public:
	CTransactionStatusRequest( uint256 const & _transactionHash, common::CClientMediumFilter * _mediumFilter );
	void accept( common::CClientBaseMedium * _medium ) const;
	common::CClientMediumFilter * getMediumFilter() const;
	uint256 m_transactionHash;
};

struct CTransactionSendRequest : public common::CRequest< common::CClientTypes >
{
public:
	CTransactionSendRequest( CTransaction const & _transaction, common::CClientMediumFilter * _mediumFilter );
	void accept( common::CClientBaseMedium * _medium ) const;
	common::CClientMediumFilter * getMediumFilter() const;
	CTransaction m_transaction;
};

struct CErrorForAppPaymentProcessing : public common::CRequest< common::CClientTypes >
{
public:
	CErrorForAppPaymentProcessing( dims::CAppError::Enum _error, common::CClientMediumFilter * _mediumFilter );
	void accept( common::CClientBaseMedium * _medium ) const;
	int m_error;
};

struct CProofTransactionAndStatusRequest : public common::CRequest< common::CClientTypes >
{
	CProofTransactionAndStatusRequest( CTransaction const & _trasaction, std::vector<unsigned char> const & _transactionStatusSignature, CPubKey const & _servicingTracker, common::CMonitorData const & _monitorData, CPubKey const & _servicingMonitor, common::CClientMediumFilter * _mediumFilter );

public:
	void accept( common::CClientBaseMedium * _medium ) const;

	common::CPayApplicationData m_payApplicationData;
};


}

#endif // CLIENT_REQUESTS_H
