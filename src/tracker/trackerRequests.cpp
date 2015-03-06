// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "trackerRequests.h"
#include "common/mediumKinds.h"
#include "trackerFilters.h"

namespace tracker
{

CDeliverInfoRequest::CDeliverInfoRequest( uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter )
	: m_actionKey( _actionKey )
	, common::CRequest< TrackerResponses >( _mediumFilter )
{
}

void
CDeliverInfoRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

common::CTrackerMediumFilter *
CDeliverInfoRequest::getMediumFilter() const
{
	return m_mediumFilter;
}


CValidateTransactionsRequest::CValidateTransactionsRequest( std::vector< CTransaction > const & _transactions, common::CTrackerMediumFilter * _mediumFilter )
	: common::CRequest< TrackerResponses >( _mediumFilter )
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
	: common::CRequest< TrackerResponses >( _mediumFilter )
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
	: common::CRequest< TrackerResponses >( _mediumFilter )
	, m_transactions( _transactions )
	, m_actionKey( _actionKey )
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
	: common::CRequest< TrackerResponses >( _mediumFilter )
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

//common::CMediumKinds::BitcoinsNodes;
CAskForTransactionsRequest::CAskForTransactionsRequest( std::vector< uint256 > const & _blockHashes, common::CTrackerMediumFilter * _mediumFilter )
	: common::CRequest< TrackerResponses >( _mediumFilter )
	, m_blockHashes( _blockHashes )
{
}

void
CAskForTransactionsRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

common::CTrackerMediumFilter *
CAskForTransactionsRequest::getMediumFilter() const
{
	return common::CRequest< TrackerResponses >::m_mediumFilter;
}

std::vector< uint256 > const &
CAskForTransactionsRequest::getBlockHashes() const
{
	return m_blockHashes;
}

CSetBloomFilterRequest::CSetBloomFilterRequest( CBloomFilter const & _bloomFilter )
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::BitcoinsNodes ) )
	, m_bloomFilter( _bloomFilter )
{
}

void
CSetBloomFilterRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

common::CTrackerMediumFilter *
CSetBloomFilterRequest::getMediumFilter() const
{
	return m_mediumFilter;
}

CBloomFilter const &
CSetBloomFilterRequest::getBloomFilter() const
{
	return m_bloomFilter;
}


CConnectToTrackerRequest::CConnectToTrackerRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress )
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_trackerAddress( _trackerAddress )
	, m_serviceAddress( _serviceAddress )
{
}

void
CConnectToTrackerRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

common::CTrackerMediumFilter *
CConnectToTrackerRequest::getMediumFilter() const
{
	return m_mediumFilter;
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


CGetSynchronizationInfoRequest::CGetSynchronizationInfoRequest( uint256 const & _actionKey, uint64_t _timeStamp )
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Trackers ) )
	, m_actionKey( _actionKey )
	, m_timeStamp( _timeStamp )
{
}

CGetSynchronizationInfoRequest::CGetSynchronizationInfoRequest( uint256 const & _actionKey, uint64_t _timeStamp, common::CTrackerMediumFilter * _mediumFilter )
	: common::CRequest< TrackerResponses >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_timeStamp( _timeStamp )
{
}

void
CGetSynchronizationInfoRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

common::CTrackerMediumFilter *
CGetSynchronizationInfoRequest::getMediumFilter() const
{
	return common::CRequest< TrackerResponses >::m_mediumFilter;
}

uint256
CGetSynchronizationInfoRequest::getActionKey() const
{
	return m_actionKey;
}

uint64_t
CGetSynchronizationInfoRequest::getTimeStamp() const
{
	return m_timeStamp;
}

CGetNextBlockRequest::CGetNextBlockRequest( uint256 const & _actionKey, common::CTrackerMediumFilter * _mediumFilter, int _blockKind  )
	: common::CRequest< TrackerResponses >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_blockKind( _blockKind )
{
}

void
CGetNextBlockRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

common::CTrackerMediumFilter *
CGetNextBlockRequest::getMediumFilter() const
{
	return common::CRequest< TrackerResponses >::m_mediumFilter;
}

uint256
CGetNextBlockRequest::getActionKey() const
{
	return m_actionKey;
}

int
CGetNextBlockRequest::getBlockKind() const
{
	return m_blockKind;
}


CGetBalanceRequest::CGetBalanceRequest( uint160 const & _key )
	: common::CRequest< TrackerResponses >( new CMediumClassFilter( common::CMediumKinds::Internal ) )
	, m_key( _key )
{
}

void
CGetBalanceRequest::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

common::CTrackerMediumFilter *
CGetBalanceRequest::getMediumFilter() const
{
	return m_mediumFilter;
}

uint160
CGetBalanceRequest::getKey() const
{
	return m_key;
}

}
