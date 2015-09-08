// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackerRequests.h"
#include "common/mediumKinds.h"
#include "trackerFilters.h"

namespace tracker
{

CDeliverInfoRequest::CDeliverInfoRequest( uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter )
	: common::CRequest< common::CTrackerTypes >( _mediumFilter )
	, m_actionKey( _actionKey )
{
}

void
CDeliverInfoRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

CValidateTransactionsRequest::CValidateTransactionsRequest( std::vector< CTransaction > const & _transactions, common::CTrackerMediumFilter * _mediumFilter )
	: common::CRequest< common::CTrackerTypes >( _mediumFilter )
	, m_transactions( _transactions )
{
}

void CValidateTransactionsRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

std::vector< CTransaction > const &
CValidateTransactionsRequest::getTransactions() const
{
	return m_transactions;
}

CPassMessageRequest::CPassMessageRequest(  common::CMessage const & _message, uint256 const & _actionKey, CPubKey const & _prevKey, common::CTrackerMediumFilter * _mediumFilter )
	: common::CRequest< common::CTrackerTypes >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_message( _message )
	, m_prevKey( _prevKey )
{
}

void
CPassMessageRequest::accept( common::CTrackerBaseMedium * _medium ) const
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

CPubKey const &
CPassMessageRequest::getPreviousKey() const
{
	return m_prevKey;
}

CTransactionsPropagationRequest::CTransactionsPropagationRequest( std::vector< CTransaction > const & _transactions, uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter )
	: common::CRequest< common::CTrackerTypes >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_transactions( _transactions )
{
}

void
CTransactionsPropagationRequest::accept( common::CTrackerBaseMedium * _medium ) const
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

CTransactionsStatusRequest::CTransactionsStatusRequest( CBundleStatus::Enum _bundleStatus, uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter )
	: common::CRequest< common::CTrackerTypes >( _mediumFilter )
	, m_actionKey( _actionKey )
	,m_bundleStatus( _bundleStatus )
{
}

void
CTransactionsStatusRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

uint256
CTransactionsStatusRequest::getActionKey() const
{
	return m_actionKey;
}

CBundleStatus::Enum
CTransactionsStatusRequest::getBundleStatus() const
{
	return m_bundleStatus;
}


CSetBloomFilterRequest::CSetBloomFilterRequest( CBloomFilter const & _bloomFilter )
	: common::CRequest< common::CTrackerTypes >( new CMediumClassFilter( common::CMediumKinds::BitcoinsNodes ) )
	, m_bloomFilter( _bloomFilter )
{
}

void
CSetBloomFilterRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

CBloomFilter const &
CSetBloomFilterRequest::getBloomFilter() const
{
	return m_bloomFilter;
}


CConnectToTrackerRequest::CConnectToTrackerRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress )
	: common::CRequest< common::CTrackerTypes >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_trackerAddress( _trackerAddress )
	, m_serviceAddress( _serviceAddress )
{
}

void
CConnectToTrackerRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}
std::string
CConnectToTrackerRequest::getAddress() const
{
	return m_trackerAddress;
}

CAddress
CConnectToTrackerRequest::getServiceAddress() const
{
	return m_serviceAddress;
}

CGetBalanceRequest::CGetBalanceRequest( uint160 const & _key )
	: common::CRequest< common::CTrackerTypes >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_key( _key )
{
}

void
CGetBalanceRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

uint160
CGetBalanceRequest::getKey() const
{
	return m_key;
}

CAskForRegistrationRequest::CAskForRegistrationRequest( uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter )
	: common::CRequest< common::CTrackerTypes >( _mediumFilter )
	, m_actionKey( _actionKey )
{
}

void
CAskForRegistrationRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

uint256
CAskForRegistrationRequest::getActionKey() const
{
	return m_actionKey;
}

CRegisterProofRequest::CRegisterProofRequest( uint256 const & _transactionHash, uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter )
	: common::CRequest< common::CTrackerTypes >( _mediumFilter )
	, m_transactionHash( _transactionHash )
	, m_actionKey( _actionKey )
{
}

void
CRegisterProofRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

uint256
CRegisterProofRequest::getActionKey() const
{
	return m_actionKey;
}

uint256
CRegisterProofRequest::getTransactionHash() const
{
	return m_transactionHash;
}

CTransactionConditionRequest::CTransactionConditionRequest( uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter )
: common::CRequest< common::CTrackerTypes >( _mediumFilter )
, m_actionKey( _actionKey )
{
}

void
CTransactionConditionRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

uint256
CTransactionConditionRequest::getActionKey() const
{
	return m_actionKey;
}

CTransactionAsClientRequest::CTransactionAsClientRequest( CTransaction const & _transaction, uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter )
	: common::CRequest< common::CTrackerTypes >( _mediumFilter )
	, m_transaction( _transaction )
	, m_actionKey( _actionKey )
{
}

void
CTransactionAsClientRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

uint256
CTransactionAsClientRequest::getActionKey() const
{
	return m_actionKey;
}

CTransaction
CTransactionAsClientRequest::getTransaction() const
{
	return m_transaction;
}

}
