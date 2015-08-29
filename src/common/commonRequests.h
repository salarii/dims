// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef COMMON_REQUESTS_H
#define COMMON_REQUESTS_H

#include "common/request.h"
#include "common/medium.h"
#include "common/filters.h"

#include "net.h"
#include "util.h"


// rename to  common requests??
namespace common
{

struct CBlockKind
{
	enum Enum
	{
		Segment
		, Header
	};
};

template < class _Types >
class CSendIdentifyDataRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CSendIdentifyDataRequest( std::vector< unsigned char > const & _signed, CPubKey const & _key, std::vector< unsigned char > const & _payload, uint256 const & _actionKey, FilterType * _mediumFilter );

	void accept( MediumType * _medium ) const;

	std::vector< unsigned char > getSigned() const;

	CPubKey getKey() const;

	std::vector< unsigned char > getPayload()const;

	uint256 getActionKey() const;
private:
	std::vector< unsigned char > m_signed;

	CPubKey m_key;

	std::vector< unsigned char > m_payload;

	uint256 const m_actionKey;
};

template < class _Types >
CSendIdentifyDataRequest< _Types >::CSendIdentifyDataRequest( std::vector< unsigned char > const & _signed, CPubKey const & _key, std::vector< unsigned char > const & _payload, uint256 const & _actionKey, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
	, m_signed( _signed )
	, m_key( _key )
	, m_payload( _payload )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CSendIdentifyDataRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
std::vector< unsigned char >
CSendIdentifyDataRequest< _Types >::getSigned() const
{
	return m_signed;
}

template < class _Types >
CPubKey
CSendIdentifyDataRequest< _Types >::getKey() const
{
	return m_key;
}

template < class _Types >
std::vector< unsigned char >
CSendIdentifyDataRequest< _Types >::getPayload()const
{
	return m_payload;
}

template < class _Types >
uint256
CSendIdentifyDataRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
class CConnectToNodeRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CConnectToNodeRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	std::string getAddress() const;

	CAddress getServiceAddress() const;
private:
	std::string const m_trackerAddress;

	CAddress const m_serviceAddress;
};

template < class _Types >
CConnectToNodeRequest< _Types >::CConnectToNodeRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
	, m_trackerAddress( _trackerAddress )
	,m_serviceAddress( _serviceAddress )
{
}

template < class _Types >
void
CConnectToNodeRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
std::string
CConnectToNodeRequest< _Types >::getAddress() const
{
	return m_trackerAddress;
}

template < class _Types >
CAddress
CConnectToNodeRequest< _Types >::getServiceAddress() const
{
	return m_serviceAddress;
}

template < class _Types >
class CNetworkRoleRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CNetworkRoleRequest( int _role, uint256 const & _actionKey, uint256 const & _id, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	int getRole() const;

	uint256 getActionKey() const;
private:
	int m_role;

	uint256 const m_actionKey;
};

template < class _Types >
CNetworkRoleRequest< _Types >::CNetworkRoleRequest( int _role, uint256 const & _actionKey, uint256 const & _id, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _id, _mediumFilter )
	, m_role( _role )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CNetworkRoleRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
int
CNetworkRoleRequest< _Types >::getRole() const
{
	return m_role;
}

template < class _Types >
uint256
CNetworkRoleRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
class CInfoAskRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CInfoAskRequest( common::CInfoKind::Enum _infoKind, uint256 const & _actionKey, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;

	common::CInfoKind::Enum getInfoKind() const;

	std::vector<unsigned char> getPayload() const;

	template < class T >
	void setPayload( T const & _payload )
	{
		common::castTypeToCharVector( &_payload, m_payLoad );
	}
private:
	uint256 const m_actionKey;

	common::CInfoKind::Enum m_infoKind;

	std::vector<unsigned char> m_payLoad;
};

template < class _Types >
CInfoAskRequest< _Types >::CInfoAskRequest( common::CInfoKind::Enum _infoKind, uint256 const & _actionKey, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
	, m_infoKind( _infoKind )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CInfoAskRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint256
CInfoAskRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
common::CInfoKind::Enum
CInfoAskRequest< _Types >::getInfoKind() const
{
	return m_infoKind;
}

template < class _Types >
std::vector<unsigned char>
CInfoAskRequest< _Types >::getPayload() const
{
	return m_payLoad;
}

template < class _Types >
class CKnownNetworkInfoRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CKnownNetworkInfoRequest( uint256 const & _actionKey, CKnownNetworkInfo const & _networkInfo, uint256 const & _id, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	CKnownNetworkInfo getNetworkInfo() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;

	CKnownNetworkInfo m_networkInfo;
};

template < class _Types >
CKnownNetworkInfoRequest< _Types >::CKnownNetworkInfoRequest( uint256 const & _actionKey, CKnownNetworkInfo const & _networkInfo, uint256 const & _id, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _id, _mediumFilter )
	, m_actionKey( _actionKey )
	, m_networkInfo( _networkInfo )
{
}

template < class _Types >
void
CKnownNetworkInfoRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
CKnownNetworkInfo
CKnownNetworkInfoRequest< _Types >::getNetworkInfo() const
{
	return m_networkInfo;
}

template < class _Types >
uint256
CKnownNetworkInfoRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
class CAckRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CAckRequest( uint256 const & _actionKey, uint256 const & _id, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

template < class _Types >
CAckRequest< _Types >::CAckRequest( uint256 const & _actionKey, uint256 const & _id, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _id, _mediumFilter )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CAckRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint256
CAckRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
class CResultRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CResultRequest( uint256 const & _actionKey, uint256 const & _id, unsigned int _result, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;

	unsigned int getResult() const{ return m_result; }
private:
	unsigned int m_result;

	uint256 const m_actionKey;
};

template < class _Types >
CResultRequest< _Types >::CResultRequest( uint256 const & _actionKey, uint256 const & _id, unsigned int _result, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _id, _mediumFilter )
	, m_result( _result )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CResultRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint256
CResultRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
class CTimeEventRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CTimeEventRequest( int64_t _requestedDelay, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	int64_t getEventTime() const;
private:
	unsigned int m_result;

	int64_t m_requestedDelay;
};

template < class _Types >
CTimeEventRequest< _Types >::CTimeEventRequest( int64_t _requestedDelay, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
	, m_requestedDelay( _requestedDelay )
{
}

template < class _Types >
void
CTimeEventRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
int64_t
CTimeEventRequest< _Types >::getEventTime() const
{
	return m_requestedDelay;
}

template < class _Types >
class CPingRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CPingRequest( uint256 const & _actionKey, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

template < class _Types >
CPingRequest< _Types >::CPingRequest( uint256 const & _actionKey, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CPingRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint256
CPingRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
class CPongRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CPongRequest( uint256 const & _actionKey, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

template < class _Types >
CPongRequest< _Types >::CPongRequest( uint256 const & _actionKey, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CPongRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint256
CPongRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
class CScheduleAbleAction;

template < class _Types >
class CScheduleActionRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CScheduleActionRequest( CScheduleAbleAction< _Types > * _scheduleAbleAction, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	CScheduleAbleAction< _Types > * getAction() const;
private:
	CScheduleAbleAction< _Types > * m_scheduleAbleAction;
};

template < class _Types >
CScheduleActionRequest< _Types >::CScheduleActionRequest( CScheduleAbleAction< _Types > * _scheduleAbleAction, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
	, m_scheduleAbleAction( _scheduleAbleAction )
{
}

template < class _Types >
void
CScheduleActionRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
CScheduleAbleAction< _Types > *
CScheduleActionRequest< _Types >::getAction() const
{
	return m_scheduleAbleAction;
}

template < class Block, class _Types >
class CSetNextBlockRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CSetNextBlockRequest( uint256 const & _actionKey, uint256 const & _id, FilterType * _mediumFilter, Block * _discBlock, unsigned int _blockIndex );

	virtual void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;

	Block * getBlock() const;

	unsigned int getBlockIndex() const;
private:
	uint256 const m_actionKey;

	Block * m_discBlock;

	unsigned int m_blockIndex;
};

template < class Block, class _Types >
CSetNextBlockRequest< Block, _Types >::CSetNextBlockRequest( uint256 const & _actionKey, uint256 const & _id, FilterType * _mediumFilter, Block * _discBlock, unsigned int _blockIndex )
	: common::CRequest< _Types >( _id, _mediumFilter )
	, m_actionKey( _actionKey )
	, m_discBlock( _discBlock )
	, m_blockIndex( _blockIndex )
{
}

template < class Block, class _Types >
void
CSetNextBlockRequest< Block, _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class Block, class _Types >
uint256
CSetNextBlockRequest< Block, _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class Block, class _Types >
Block *
CSetNextBlockRequest< Block, _Types >::getBlock() const
{
	return m_discBlock;
}

template < class Block, class _Types >
unsigned int
CSetNextBlockRequest< Block, _Types >::getBlockIndex() const
{
	return m_blockIndex;
}

template < class _Types >
class CAskForTransactionsRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CAskForTransactionsRequest( std::vector< uint256 > const & _blockHashes, FilterType * _mediumFilter );

	virtual void accept( MediumType * m_mediumNumber ) const;

	std::vector< uint256 > const & getBlockHashes() const;
private:
	std::vector< uint256 > const m_blockHashes;

};

template < class _Types >
CAskForTransactionsRequest< _Types >::CAskForTransactionsRequest( std::vector< uint256 > const & _blockHashes, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
	, m_blockHashes( _blockHashes )
{
}

template < class _Types >
void
CAskForTransactionsRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
std::vector< uint256 > const &
CAskForTransactionsRequest< _Types >::getBlockHashes() const
{
	return m_blockHashes;
}

template < class _Types >
class CSetBloomFilterRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CSetBloomFilterRequest( CBloomFilter const & _bloomFilter, FilterType * _filterType );

	virtual void accept( MediumType * _medium ) const;

	CBloomFilter const & getBloomFilter() const;
private:
	CBloomFilter const m_bloomFilter;
};

template < class _Types >
CSetBloomFilterRequest< _Types >::CSetBloomFilterRequest( CBloomFilter const & _bloomFilter, FilterType * _filterType )
	: common::CRequest< _Types >( _filterType )
	, m_bloomFilter( _bloomFilter )
{
}

template < class _Types >
void
CSetBloomFilterRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
CBloomFilter const &
CSetBloomFilterRequest< _Types >::getBloomFilter() const
{
	return m_bloomFilter;
}

template < class _Types >
class CBalanceRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CBalanceRequest( FilterType * _filterType );

	virtual void accept( MediumType * _medium ) const;

	uint160 getKey() const;
private:
	uint160 m_key;
};

template < class _Types >
CBalanceRequest< _Types >::CBalanceRequest( FilterType * _filterType )
	: common::CRequest< _Types >( _filterType )
{
}

template < class _Types >
void
CBalanceRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint160
CBalanceRequest< _Types >::getKey() const
{
	return m_key;
}


template < class _Types >
class CValidRegistrationRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CValidRegistrationRequest( CPubKey const & _key, uint64_t const _contractTime, uint64_t const _period, uint256 const _actionKey, uint256 const & _id, FilterType * _filterType );

	virtual void accept( MediumType * _medium ) const;

	CPubKey const & getKey() const;

	uint64_t getPeriod() const;

	uint64_t getContractTime() const;

	uint256 getActionKey() const;
private:
	CPubKey const m_key;

	uint64_t const m_contractTime;

	uint64_t const m_period;

	uint256 const m_actionKey;
};

template < class _Types >
CValidRegistrationRequest< _Types >::CValidRegistrationRequest( CPubKey const & _key, uint64_t const _contractTime, uint64_t const _period, uint256 const _actionKey, uint256 const & _id, FilterType * _filterType )
	: common::CRequest< _Types >( _id, _filterType )
	, m_key( _key )
	, m_contractTime( _contractTime )
	, m_period( _period )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CValidRegistrationRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
CPubKey const &
CValidRegistrationRequest< _Types >::getKey() const
{
	return m_key;
}

template < class _Types >
uint256
CValidRegistrationRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
uint64_t
CValidRegistrationRequest< _Types >::getPeriod() const
{
	return m_period;
}

template < class _Types >
uint64_t
CValidRegistrationRequest< _Types >::getContractTime() const
{
	return m_contractTime;
}

template < class _Types >
class CStorageInfoRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CStorageInfoRequest( uint64_t const _time, uint64_t const _storageSize, uint64_t const _headerSize, uint256 const & _actionKey, uint256 const & _id, FilterType * _filterType );

	virtual void accept( MediumType * _medium ) const;

	uint64_t getTime() const;

	uint64_t getStorageSize() const;

	uint64_t getHeaderSize() const;

	uint256 getActionKey() const;
private:
	uint64_t const m_time;

	uint64_t const m_storageSize;

	uint64_t const m_headerSize;

	uint256 const m_actionKey;
};

template < class _Types >
CStorageInfoRequest< _Types >::CStorageInfoRequest( uint64_t const _time, uint64_t const _storageSize, uint64_t const _headerSize, uint256 const & _actionKey, uint256 const & _id, FilterType * _filterType )
	: common::CRequest< _Types >( _id, _filterType )
	, m_time( _time )
	, m_storageSize( _storageSize )
	, m_headerSize( _headerSize )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CStorageInfoRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint64_t
CStorageInfoRequest< _Types >::getStorageSize() const
{
	return m_storageSize;
}

template < class _Types >
uint64_t
CStorageInfoRequest< _Types >::getHeaderSize() const
{
	return m_headerSize;
}

template < class _Types >
uint64_t
CStorageInfoRequest< _Types >::getTime() const
{
	return m_time;
}

template < class _Types >
uint256
CStorageInfoRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
class CGetBlockRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CGetBlockRequest( unsigned int _blockNumber, int _blockKind, uint256 const & _actionKey, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;

	int getBlockKind() const;

	int getBlockNumber() const;
private:
	unsigned int m_blockNumber;

	int m_blockKind;

	uint256 const m_actionKey;
};

template < class _Types >
CGetBlockRequest< _Types >::CGetBlockRequest( unsigned int _blockNumber, int _blockKind, uint256 const & _actionKey, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
	, m_blockNumber( _blockNumber )
	, m_blockKind( _blockKind )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CGetBlockRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint256
CGetBlockRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
int
CGetBlockRequest< _Types >::getBlockKind() const
{
	return m_blockKind;
}

template < class _Types >
int
CGetBlockRequest< _Types >::getBlockNumber() const
{
	return m_blockNumber;
}

template < class _Types >
class CGetSynchronizationInfoRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CGetSynchronizationInfoRequest( uint256 const & _actionKey, uint64_t _timeStamp, FilterType * _filterType );

	virtual void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;

	uint64_t getTimeStamp() const;
private:
	uint256 const m_actionKey;

	uint64_t const m_timeStamp;
};

template < class _Types >
CGetSynchronizationInfoRequest< _Types >::CGetSynchronizationInfoRequest( uint256 const & _actionKey, uint64_t _timeStamp, FilterType * _filterType )
	: common::CRequest< _Types >( _filterType )
	, m_actionKey( _actionKey )
	, m_timeStamp( _timeStamp )
{
}

template < class _Types >
void
CGetSynchronizationInfoRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint256
CGetSynchronizationInfoRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
uint64_t
CGetSynchronizationInfoRequest< _Types >::getTimeStamp() const
{
	return m_timeStamp;
}

template < class _Types >
class CSynchronizationRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CSynchronizationRequest( uint256 const & _actionKey, FilterType * _filterType );

	virtual void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

template < class _Types >
CSynchronizationRequest< _Types >::CSynchronizationRequest( uint256 const & _actionKey, FilterType * _filterType )
	: common::CRequest< _Types >( _filterType )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CSynchronizationRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint256
CSynchronizationRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
class CBitcoinHeaderRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CBitcoinHeaderRequest( CBlockHeader const & _blockHeader, uint256 const & _actionKey, uint256 const & _id, FilterType * _filterType );

	virtual void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;

	CBlockHeader const & getBlockHeader() const
	{
		return m_blockHeader;
	}
private:
	CBlockHeader m_blockHeader;
	uint256 const m_actionKey;
};

template < class _Types >
CBitcoinHeaderRequest< _Types >::CBitcoinHeaderRequest( CBlockHeader const & _blockHeader, uint256 const & _actionKey, uint256 const & _id, FilterType * _filterType )
	: common::CRequest< _Types >( _id, _filterType )
	, m_blockHeader( _blockHeader )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CBitcoinHeaderRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint256
CBitcoinHeaderRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
class CGetBalanceRequest: public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CGetBalanceRequest( uint160 const & _key, uint256 const & _actionKey, uint256 const & _id, FilterType * _filterType );

	virtual void accept( common::CTrackerBaseMedium * _medium ) const;

	uint256 getActionKey() const;

	uint160 getKey() const;
private:
	uint160 m_key;
	uint256 const m_actionKey;
};

template < class _Types >
CGetBalanceRequest< _Types >::CGetBalanceRequest( uint160 const & _key, uint256 const & _actionKey, uint256 const & _id, FilterType * _filterType )
	: common::CRequest< _Types >( _id, _filterType )
	, m_key( _key )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CGetBalanceRequest< _Types >::accept( common::CTrackerBaseMedium * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint256
CGetBalanceRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
uint160
CGetBalanceRequest< _Types >::getKey() const
{
	return m_key;
}

}

#endif // COMMON_REQUESTS_H
