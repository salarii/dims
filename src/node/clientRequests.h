// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLIENTREQUESTS_H
#define CLIENTREQUESTS_H

#include "configureNodeActionHadler.h"

#include "common/request.h"

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
	CTransactionStatusRequest( uint256 const & _transactionHash, common::CMediumFilter< NodeResponses > * _medium );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;
	uint256 m_transactionHash;
};

struct CTransactionSendRequest : public common::CRequest< NodeResponses >
{
public:
	CTransactionSendRequest( CTransaction const & _transaction, common::CMediumFilter< NodeResponses > * _medium );
	void accept( common::CMedium< NodeResponses > * _medium ) const;
	common::CMediumFilter< NodeResponses > * getMediumFilter() const;
	CTransaction m_transaction;

};

}

#endif // CLIENTREQUESTS_H
