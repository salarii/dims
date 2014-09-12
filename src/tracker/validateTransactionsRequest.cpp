// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "validateTransactionsRequest.h"
#include "common/medium.h"
#include "common/mediumKinds.h"
#include "trackerFilters.h"

namespace tracker
{

CValidateTransactionsRequest::CValidateTransactionsRequest( std::vector< CTransaction > const & _transactions, common::CMediumFilter< TrackerResponses > * _mediumFilter )
	: common::CRequest< TrackerResponses >( _mediumFilter )
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

CPassMessageRequest::CPassMessageRequest(  common::CMessage const & _message, uint256 const & _actionKey, common::CMediumFilter< TrackerResponses > * _mediumFilter )
	: common::CRequest< TrackerResponses >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_message( _message )
{
}

void
CPassMessageRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

common::CMessage const &
CPassMessageRequest::getMessage() const
{
	return m_message;
}

uint256
CPassMessageRequest::getActionKey() const
{
	return m_actionKey;
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

CTransactionsStatusRequest::CTransactionsStatusRequest( CBundleStatus::Enum _bundleStatus, uint256 const & _actionKey, common::CMediumFilter< TrackerResponses > * _mediumFilter )
	: common::CRequest< TrackerResponses >( _mediumFilter )
	, m_actionKey( _actionKey )
	,m_bundleStatus( _bundleStatus )
{
}

void
CTransactionsStatusRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

uint256
CTransactionsStatusRequest::getActionKey() const
{
	return m_actionKey;
}

}
