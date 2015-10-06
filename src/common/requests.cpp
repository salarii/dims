// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/requests.h"
#include "common/medium.h"

namespace common
{

CSendIdentifyDataRequest::CSendIdentifyDataRequest( std::vector< unsigned char > const & _signed, CPubKey const & _key, std::vector< unsigned char > const & _payload, uint256 const & _actionKey, CMediumFilter * _mediumFilter )
	: common::CRequest( _mediumFilter )
	, m_signed( _signed )
	, m_key( _key )
	, m_payload( _payload )
	, m_actionKey( _actionKey )
{
}

void
CSendIdentifyDataRequest::accept( CMedium * _medium ) const
{
	_medium->add( this );
}

std::vector< unsigned char >
CSendIdentifyDataRequest::getSigned() const
{
	return m_signed;
}

CPubKey
CSendIdentifyDataRequest::getKey() const
{
	return m_key;
}

std::vector< unsigned char >
CSendIdentifyDataRequest::getPayload()const
{
	return m_payload;
}

uint256
CSendIdentifyDataRequest::getActionKey() const
{
	return m_actionKey;
}

CConnectToNodeRequest::CConnectToNodeRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress, CMediumFilter * _mediumFilter )
	: common::CRequest( _mediumFilter )
	, m_trackerAddress( _trackerAddress )
	,m_serviceAddress( _serviceAddress )
{
}

void
CConnectToNodeRequest::accept( CMedium * _medium ) const
{
	_medium->add( this );
}

std::string
CConnectToNodeRequest::getAddress() const
{
	return m_trackerAddress;
}

CAddress
CConnectToNodeRequest::getServiceAddress() const
{
	return m_serviceAddress;
}

CInfoAskRequest::CInfoAskRequest( common::CInfoKind::Enum _infoKind, uint256 const & _actionKey, CMediumFilter * _mediumFilter )
	: common::CRequest( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_infoKind( _infoKind )
{
}

void
CInfoAskRequest::accept( CMedium * _medium ) const
{
	_medium->add( this );
}

uint256
CInfoAskRequest::getActionKey() const
{
	return m_actionKey;
}

common::CInfoKind::Enum
CInfoAskRequest::getInfoKind() const
{
	return m_infoKind;
}

std::vector<unsigned char>
CInfoAskRequest::getPayload() const
{
	return m_payLoad;
}

CAckRequest::CAckRequest( uint256 const & _actionKey, uint256 const & _id, CMediumFilter * _mediumFilter )
	: common::CRequest( _id, _mediumFilter )
	, m_actionKey( _actionKey )
{
}

void
CAckRequest::accept( CMedium * _medium ) const
{
	_medium->add( this );
}

uint256
CAckRequest::getActionKey() const
{
	return m_actionKey;
}

CTimeEventRequest::CTimeEventRequest( int64_t _requestedDelay, CMediumFilter * _mediumFilter )
	: common::CRequest( _mediumFilter )
	, m_requestedDelay( _requestedDelay )
{
}

void
CTimeEventRequest::accept( CMedium * _medium ) const
{
	_medium->add( this );
}

int64_t
CTimeEventRequest::getEventTime() const
{
	return m_requestedDelay;
}

CScheduleActionRequest::CScheduleActionRequest( CScheduleAbleAction * _scheduleAbleAction, CMediumFilter * _mediumFilter )
	: common::CRequest( _mediumFilter )
	, m_scheduleAbleAction( _scheduleAbleAction )
{
}

void
CScheduleActionRequest::accept( CMedium * _medium ) const
{
	_medium->add( this );
}

CScheduleAbleAction *
CScheduleActionRequest::getAction() const
{
	return m_scheduleAbleAction;
}

CAskForTransactionsRequest::CAskForTransactionsRequest( std::vector< uint256 > const & _blockHashes, CMediumFilter * _mediumFilter )
	: common::CRequest( _mediumFilter )
	, m_blockHashes( _blockHashes )
{
}

void
CAskForTransactionsRequest::accept( CMedium * _medium ) const
{
	_medium->add( this );
}

std::vector< uint256 > const &
CAskForTransactionsRequest::getBlockHashes() const
{
	return m_blockHashes;
}

CSetBloomFilterRequest::CSetBloomFilterRequest( CBloomFilter const & _bloomFilter, CMediumFilter * _CMediumFilter )
	: common::CRequest( _CMediumFilter )
	, m_bloomFilter( _bloomFilter )
{
}

void
CSetBloomFilterRequest::accept( CMedium * _medium ) const
{
	_medium->add( this );
}

CBloomFilter const &
CSetBloomFilterRequest::getBloomFilter() const
{
	return m_bloomFilter;
}

CBalanceRequest::CBalanceRequest( CMediumFilter * _CMediumFilter )
	: common::CRequest( _CMediumFilter )
{
}

uint160
CBalanceRequest::getKey() const
{
	return m_key;
}

void
CBalanceRequest::accept( CMedium * _medium ) const
{
	_medium->add( this );
}

CSendMessageRequest::CSendMessageRequest( CPayloadKind::Enum _messageKind, uint256 const & _actionKey, uint256 const & _id, CMediumFilter * _CMediumFilter )
	: common::CRequest( _id, _CMediumFilter )
	, m_messageKind( (int)_messageKind )
	, m_actionKey( _actionKey )
	, m_payload( std::vector< unsigned char >() )
{
}

CSendMessageRequest::CSendMessageRequest( CPayloadKind::Enum _messageKind, uint256 const & _actionKey, CMediumFilter * _CMediumFilter )
	: common::CRequest( _CMediumFilter )
	, m_messageKind( (int)_messageKind )
	, m_actionKey( _actionKey )
	, m_payload( std::vector< unsigned char >() )
{
}

CSendMessageRequest::CSendMessageRequest(
		common::CMainRequestType::Enum _messageKind
		, CMediumFilter * _CMediumFilter )
	: common::CRequest( _CMediumFilter )
	, m_messageKind( (int)_messageKind )
	, m_payload( std::vector< unsigned char >() )
{}

CSendMessageRequest::CSendMessageRequest(
		common::CMainRequestType::Enum _messageKind
		, uint256 const & _id
		, CMediumFilter * _CMediumFilter )
	: common::CRequest( _id, _CMediumFilter )
	, m_messageKind( (int)_messageKind )
	, m_payload( std::vector< unsigned char >() )
{}

void
CSendMessageRequest::accept( CMedium * _medium ) const
{
	_medium->add( this );
}

uint256
CSendMessageRequest::getActionKey() const
{
	return m_actionKey;
}

int
CSendMessageRequest::getMessageKind() const
{
	return m_messageKind;
}

std::vector< unsigned char > const &
CSendMessageRequest::getPayLoad() const
{
	return m_payload;
}

}
