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

template < class ResponsesType >
class CSendIdentifyDataRequest : public common::CRequest< ResponsesType >
{
public:
	CSendIdentifyDataRequest( std::vector< unsigned char > const & _signed, CPubKey const & _key, std::vector< unsigned char > const & _payload, uint256 const & _actionKey, common::CMediumFilter< ResponsesType > * _mediumFilter );

	void accept( common::CMedium< ResponsesType > * _medium ) const;

	common::CMediumFilter< ResponsesType > * getMediumFilter() const;

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

template < class ResponsesType >
CSendIdentifyDataRequest< ResponsesType >::CSendIdentifyDataRequest( std::vector< unsigned char > const & _signed, CPubKey const & _key, std::vector< unsigned char > const & _payload, uint256 const & _actionKey, common::CMediumFilter< ResponsesType > * _mediumFilter )
	: common::CRequest< ResponsesType >( _mediumFilter )// new common::CMediumFilter< ResponsesType >( -1, -1, new CAcceptFilterByPtr< ResponsesType >( _ptr ) )
	, m_signed( _signed )
	, m_key( _key )
	, m_payload( _payload )
	, m_actionKey( _actionKey )
{
}

template < class ResponsesType >
void
CSendIdentifyDataRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
common::CMediumFilter< ResponsesType > *
CSendIdentifyDataRequest< ResponsesType >::getMediumFilter() const
{
	return common::CRequest< ResponsesType >::m_mediumFilter;
}

template < class ResponsesType >
std::vector< unsigned char >
CSendIdentifyDataRequest< ResponsesType >::getSigned() const
{
	return m_signed;
}

template < class ResponsesType >
CPubKey
CSendIdentifyDataRequest< ResponsesType >::getKey() const
{
	return m_key;
}

template < class ResponsesType >
std::vector< unsigned char >
CSendIdentifyDataRequest< ResponsesType >::getPayload()const
{
	return m_payload;
}

template < class ResponsesType >
uint256
CSendIdentifyDataRequest< ResponsesType >::getActionKey() const
{
	return m_actionKey;
}

template < class ResponsesType >
class CConnectToNodeRequest : public common::CRequest< ResponsesType >
{
public:
	CConnectToNodeRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress, common::CMediumFilter< ResponsesType > * _mediumFilter );

	virtual void accept( common::CMedium< ResponsesType > * _medium ) const;

	virtual common::CMediumFilter< ResponsesType > * getMediumFilter() const;

	std::string getAddress() const;

	CAddress getServiceAddress() const;
private:
	std::string const m_trackerAddress;

	CAddress const m_serviceAddress;

	uint256 const m_actionKey;
};

template < class ResponsesType >
CConnectToNodeRequest< ResponsesType >::CConnectToNodeRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress, common::CMediumFilter< ResponsesType > * _mediumFilter )
	:m_trackerAddress( _trackerAddress )
	,m_serviceAddress( _serviceAddress )
	,common::CRequest< ResponsesType >( _mediumFilter )
{
}

template < class ResponsesType >
void
CConnectToNodeRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
common::CMediumFilter< ResponsesType > *
CConnectToNodeRequest< ResponsesType >::getMediumFilter() const
{
	return common::CRequest< ResponsesType >::m_mediumFilter;
}

template < class ResponsesType >
std::string
CConnectToNodeRequest< ResponsesType >::getAddress() const
{
	return m_trackerAddress;
}

template < class ResponsesType >
CAddress
CConnectToNodeRequest< ResponsesType >::getServiceAddress() const
{
	return m_serviceAddress;
}

template < class ResponsesType >
class CNetworkRoleRequest : public common::CRequest< ResponsesType >
{
public:
	CNetworkRoleRequest( uint256 const & _actionKey, int _role, common::CMediumFilter< ResponsesType > * _mediumFilter );

	virtual void accept( common::CMedium< ResponsesType > * _medium ) const;

	virtual common::CMediumFilter< ResponsesType > * getMediumFilter() const;

	int getRole() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;

	int m_role;
};

template < class ResponsesType >
CNetworkRoleRequest< ResponsesType >::CNetworkRoleRequest( uint256 const & _actionKey, int _role, common::CMediumFilter< ResponsesType > * _mediumFilter )
	: common::CRequest< ResponsesType >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_role( _role )
{
}

template < class ResponsesType >
void
CNetworkRoleRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
common::CMediumFilter< ResponsesType > *
CNetworkRoleRequest< ResponsesType >::getMediumFilter() const
{
	return common::CRequest< ResponsesType >::m_mediumFilter;
}

template < class ResponsesType >
int
CNetworkRoleRequest< ResponsesType >::getRole() const
{
	return m_role;
}

template < class ResponsesType >
uint256
CNetworkRoleRequest< ResponsesType >::getActionKey() const
{
	return m_actionKey;
}

template < class ResponsesType >
class CKnownNetworkInfoRequest : public common::CRequest< ResponsesType >
{
public:
	CKnownNetworkInfoRequest( uint256 const & _actionKey, CKnownNetworkInfo const & _networkInfo, common::CMediumFilter< ResponsesType > * _mediumFilter );

	virtual void accept( common::CMedium< ResponsesType > * _medium ) const;

	virtual common::CMediumFilter< ResponsesType > * getMediumFilter() const;

	CKnownNetworkInfo getNetworkInfo() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;

	CKnownNetworkInfo m_networkInfo;
};

template < class ResponsesType >
CKnownNetworkInfoRequest< ResponsesType >::CKnownNetworkInfoRequest( uint256 const & _actionKey, CKnownNetworkInfo const & _networkInfo, common::CMediumFilter< ResponsesType > * _mediumFilter )
	: common::CRequest< ResponsesType >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_networkInfo( _networkInfo )
{
}

template < class ResponsesType >
void
CKnownNetworkInfoRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
common::CMediumFilter< ResponsesType > *
CKnownNetworkInfoRequest< ResponsesType >::getMediumFilter() const
{
	return common::CRequest< ResponsesType >::m_mediumFilter;
}

template < class ResponsesType >
CKnownNetworkInfo
CKnownNetworkInfoRequest< ResponsesType >::getNetworkInfo() const
{
	return m_networkInfo;
}

template < class ResponsesType >
uint256
CKnownNetworkInfoRequest< ResponsesType >::getActionKey() const
{
	return m_actionKey;
}

template < class ResponsesType >
class CAckRequest : public common::CRequest< ResponsesType >
{
public:
	CAckRequest( uint256 const & _actionKey, common::CMediumFilter< ResponsesType > * _mediumFilter );

	virtual void accept( common::CMedium< ResponsesType > * _medium ) const;

	virtual common::CMediumFilter< ResponsesType > * getMediumFilter() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

template < class ResponsesType >
CAckRequest< ResponsesType >::CAckRequest( uint256 const & _actionKey, common::CMediumFilter< ResponsesType > * _mediumFilter )
	: common::CRequest< ResponsesType >( _mediumFilter )
	, m_actionKey( _actionKey )
{
}

template < class ResponsesType >
void
CAckRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
common::CMediumFilter< ResponsesType > *
CAckRequest< ResponsesType >::getMediumFilter() const
{
	return common::CRequest< ResponsesType >::m_mediumFilter;
}

template < class ResponsesType >
uint256
CAckRequest< ResponsesType >::getActionKey() const
{
	return m_actionKey;
}

template < class ResponsesType >
class CEndRequest : public common::CRequest< ResponsesType >
{
public:
	CEndRequest( uint256 const & _actionKey, common::CMediumFilter< ResponsesType > * _mediumFilter );

	void accept( common::CMedium< ResponsesType > * _medium ) const;

	common::CMediumFilter< ResponsesType > * getMediumFilter() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;
};

template < class ResponsesType >
CEndRequest< ResponsesType >::CEndRequest( uint256 const & _actionKey, common::CMediumFilter< ResponsesType > * _mediumFilter )
	: common::CRequest< ResponsesType >( _mediumFilter )
	, m_actionKey( _actionKey )
{
}

template < class ResponsesType >
void
CEndRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
common::CMediumFilter< ResponsesType > *
CEndRequest< ResponsesType >::getMediumFilter() const
{
	return common::CRequest< ResponsesType >::m_mediumFilter;
}

template < class ResponsesType >
uint256
CEndRequest< ResponsesType >::getActionKey() const
{
	return m_actionKey;
}

template < class ResponsesType >
class CResultRequest : public common::CRequest< ResponsesType >
{
public:
	CResultRequest( uint256 const & _actionKey, unsigned int _result,common::CMediumFilter< ResponsesType > * _mediumFilter );

	virtual void accept( common::CMedium< ResponsesType > * _medium ) const;

	virtual common::CMediumFilter< ResponsesType > * getMediumFilter() const;

	uint256 getActionKey() const;
private:
	unsigned int m_result;

	uint256 const m_actionKey;
};

template < class ResponsesType >
CResultRequest< ResponsesType >::CResultRequest( uint256 const & _actionKey, unsigned int _result, common::CMediumFilter< ResponsesType > * _mediumFilter )
	: common::CRequest< ResponsesType >( _mediumFilter )
	, m_actionKey( _actionKey )
	, m_result( _result )
{
}

template < class ResponsesType >
void
CResultRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
common::CMediumFilter< ResponsesType > *
CResultRequest< ResponsesType >::getMediumFilter() const
{
	return common::CRequest< ResponsesType >::m_mediumFilter;
}

template < class ResponsesType >
uint256
CResultRequest< ResponsesType >::getActionKey() const
{
	return m_actionKey;
}

template < class ResponsesType >
class CTimeEventRequest : public common::CRequest< ResponsesType >
{
public:
	CTimeEventRequest( int64_t _requestedDelay, common::CMediumFilter< ResponsesType > * _mediumFilter );

	virtual void accept( common::CMedium< ResponsesType > * _medium ) const;

	int64_t getEventTime() const;
private:
	unsigned int m_result;

	int64_t m_requestedDelay;
};

template < class ResponsesType >
CTimeEventRequest< ResponsesType >::CTimeEventRequest( int64_t _requestedDelay, common::CMediumFilter< ResponsesType > * _mediumFilter )
	: common::CRequest< ResponsesType >( _mediumFilter )
	, m_requestedDelay( _requestedDelay )
{
}

template < class ResponsesType >
void
CTimeEventRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
int64_t
CTimeEventRequest< ResponsesType >::getEventTime() const
{
	return m_requestedDelay;
}

}

#endif // MEDIUM_REQUESTS_H
