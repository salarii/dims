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

template < class _Types >
class CSendMessageRequest: public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CSendMessageRequest(
			CPayloadKind::Enum _messageKind
			, uint256 const & _actionKey
			, uint256 const & _id
			, FilterType * _filterType );

	CSendMessageRequest(
			CPayloadKind::Enum _messageKind
			, uint256 const & _actionKey
			, FilterType * _filterType );

	void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;

	int getMessageKind() const;

	std::vector< unsigned char > const & getPayLoad() const;

	//better  define it  here
	template < class T >
	void addPayload( T const & _t )
	{
		unsigned int initiaSize = m_payload.size();
		unsigned int size = ::GetSerializeSize( _t, SER_NETWORK, PROTOCOL_VERSION );
		m_payload.resize( size + initiaSize );
		CBufferAsStream stream( (char*)&m_payload[ initiaSize ], size, SER_NETWORK, PROTOCOL_VERSION );
		stream << _t;
	}

	template < class T1, class T2 >
	void addPayload( T1 const & _t1, T2 const & _t2 )
	{
		addPayload( _t1 );
		addPayload( _t2 );
	}

	template < class T1, class T2, class T3 >
	void createPayload( T1 const & _t1, T2 const & _t2, T3 const & _t3 )
	{
		addPayload( _t1, _t2 );
		addPayload( _t3 );
	}
private:
	int m_messageKind;

	uint256 const m_actionKey;

	std::vector< unsigned char > m_payload;
};

template < class _Types >
CSendMessageRequest< _Types >::CSendMessageRequest( CPayloadKind::Enum _messageKind, uint256 const & _actionKey, uint256 const & _id, FilterType * _filterType )
	: common::CRequest< _Types >( _id, _filterType )
	, m_messageKind( (int)_messageKind )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
CSendMessageRequest< _Types >::CSendMessageRequest( CPayloadKind::Enum _messageKind, uint256 const & _actionKey, FilterType * _filterType )
	: common::CRequest< _Types >( _filterType )
	, m_messageKind( (int)_messageKind )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CSendMessageRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint256
CSendMessageRequest< _Types >::getActionKey() const
{
	return m_actionKey;
}

template < class _Types >
int
CSendMessageRequest< _Types >::getMessageKind() const
{
	return m_messageKind;
}

template < class _Types >
std::vector< unsigned char > const &
CSendMessageRequest< _Types >::getPayLoad() const
{
	return m_payload;
}

}

#endif // COMMON_REQUESTS_H
