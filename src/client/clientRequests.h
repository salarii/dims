// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLIENT_REQUESTS_H
#define CLIENT_REQUESTS_H

#include "configureNodeActionHadler.h"

#include "common/request.h"
#include "common/commonResponses.h"
#include "appLib/messageType.h"
namespace client
{

struct CInfoRequestContinue : public common::CRequest< NodeResponses >
{
public:
	CInfoRequestContinue( uint256 const & _token, common::CMediumFilter< NodeResponses > * _mediumFilter );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;

	uint256 const m_token;
};

struct CTransactionStatusRequest : public common::CRequest< NodeResponses >
{
public:
	CTransactionStatusRequest( uint256 const & _transactionHash, common::CMediumFilter< NodeResponses > * _mediumFilter );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;
	uint256 m_transactionHash;
};

struct CTransactionSendRequest : public common::CRequest< NodeResponses >
{
public:
	CTransactionSendRequest( CTransaction const & _transaction, common::CMediumFilter< NodeResponses > * _mediumFilter );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;
	CTransaction m_transaction;
};

struct CErrorForAppPaymentProcessing : public common::CRequest< NodeResponses >
{
public:
	CErrorForAppPaymentProcessing( dims::CAppError::Enum _error, common::CMediumFilter< NodeResponses > * _mediumFilter );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	int m_error;
};

struct CProofTransactionAndStatusRequest : public common::CRequest< NodeResponses >
{
	CProofTransactionAndStatusRequest( CTransaction const & _trasaction, std::vector<unsigned char> const & _transactionStatusSignature, CPubKey const & _servicingTracker, common::CMonitorData const & _monitorData, CPubKey const & _servicingMonitor, common::CMediumFilter< NodeResponses > * _mediumFilter );

public:
	void accept( common::CMedium< NodeResponses > * _medium ) const;

	common::CPayApplicationData m_payApplicationData;
};


}

#endif // CLIENT_REQUESTS_H
