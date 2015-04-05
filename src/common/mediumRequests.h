// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MEDIUM_REQUESTS_H
#define MEDIUM_REQUESTS_H

#include "common/request.h"
#include "common/medium.h"
#include "common/filters.h"

#include "net.h"
#include "util.h"


// rename to  common requests??
namespace common
{

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
	: common::CRequest< _Types >( _mediumFilter )// new FILTER_TYPE(_Types)( -1, -1, new CAcceptFilterByPtr< _Types >( _ptr ) )
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

	uint256 const m_actionKey;
};

template < class _Types >
CConnectToNodeRequest< _Types >::CConnectToNodeRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress, FilterType * _mediumFilter )
	:m_trackerAddress( _trackerAddress )
	,m_serviceAddress( _serviceAddress )
	,common::CRequest< _Types >( _mediumFilter )
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
	CNetworkRoleRequest( uint256 const & _actionKey, int _role, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	int getRole() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;

	int m_role;
};

template < class _Types >
CNetworkRoleRequest< _Types >::CNetworkRoleRequest( uint256 const & _actionKey, int _role, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_role( _role )
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
class CKnownNetworkInfoRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CKnownNetworkInfoRequest( uint256 const & _actionKey, CKnownNetworkInfo const & _networkInfo, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	CKnownNetworkInfo getNetworkInfo() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;

	CKnownNetworkInfo m_networkInfo;
};

template < class _Types >
CKnownNetworkInfoRequest< _Types >::CKnownNetworkInfoRequest( uint256 const & _actionKey, CKnownNetworkInfo const & _networkInfo, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
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
	CAckRequest( uint256 const & _actionKey, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

template < class _Types >
CAckRequest< _Types >::CAckRequest( uint256 const & _actionKey, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
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
class CEndRequest : public common::CRequest< _Types >
{
public:
	using typename CRequest< _Types >::MediumType;
	using typename CRequest< _Types >::FilterType;
public:
	CEndRequest( uint256 const & _actionKey, FilterType * _mediumFilter );

	void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

template < class _Types >
CEndRequest< _Types >::CEndRequest( uint256 const & _actionKey, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
	, m_actionKey( _actionKey )
{
}

template < class _Types >
void
CEndRequest< _Types >::accept( MediumType * _medium ) const
{
	_medium->add( this );
}

template < class _Types >
uint256
CEndRequest< _Types >::getActionKey() const
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
	CResultRequest( uint256 const & _actionKey, unsigned int _result, FilterType * _mediumFilter );

	virtual void accept( MediumType * _medium ) const;

	uint256 getActionKey() const;

	unsigned int getResult() const{ return m_result; }
private:
	unsigned int m_result;

	uint256 const m_actionKey;
};

template < class _Types >
CResultRequest< _Types >::CResultRequest( uint256 const & _actionKey, unsigned int _result, FilterType * _mediumFilter )
	: common::CRequest< _Types >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_result( _result )
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

}

#endif // MEDIUM_REQUESTS_H
