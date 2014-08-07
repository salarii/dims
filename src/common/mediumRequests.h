// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MEDIUM_REQUESTS_H
#define MEDIUM_REQUESTS_H

#include "common/request.h"
#include "common/medium.h"
#include "common/filters.h"

#include "net.h"
#include "util.h"

namespace common
{

template < class ResponsesType >
class CIdentifyRequest : public common::CRequest< ResponsesType >
{
public:
	CIdentifyRequest( int _shortPtr, std::vector< unsigned char > const & _payload, uint256 const & _actionKey );

	~CIdentifyRequest();

	void accept( common::CMedium< ResponsesType > * _medium ) const;

	common::CMediumFilter< ResponsesType > * getMediumFilter() const;

	std::vector< unsigned char > getPayload() const;

	uint256 getActionKey() const;
private:

	std::vector< unsigned char > m_payload;

	common::CMediumFilter< ResponsesType > * m_mediumFilter;

	uint256 const m_actionKey;
};

template < class ResponsesType >
CIdentifyRequest< ResponsesType >::CIdentifyRequest( int _shortPtr, std::vector< unsigned char > const & _payload, uint256 const & _actionKey )
	: m_mediumFilter( new common::CMediumFilter< ResponsesType >( -1, -1, new CAcceptFilterByShortPtr< ResponsesType >( _shortPtr ) ) )
	, m_payload( _payload )
	, m_actionKey( _actionKey )
{
}

template < class ResponsesType >
uint256
CIdentifyRequest< ResponsesType >::getActionKey() const
{
	return m_actionKey;
}

template < class ResponsesType >
CIdentifyRequest< ResponsesType >::~CIdentifyRequest()
{
	delete m_mediumFilter;
}

template < class ResponsesType >
void
CIdentifyRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
common::CMediumFilter< ResponsesType > *
CIdentifyRequest< ResponsesType >::getMediumFilter() const
{
	return m_mediumFilter;
}

template < class ResponsesType >
std::vector< unsigned char >
CIdentifyRequest< ResponsesType >::getPayload() const
{
	return m_payload;
}


template < class ResponsesType >
class CIdentifyResponse : public common::CRequest< ResponsesType >
{
public:
	CIdentifyResponse( unsigned int _kind, std::vector< unsigned char > const & _signed, CPubKey const & _key, std::vector< unsigned char > const & _payload, uint256 const & _actionKey );

	void accept( common::CMedium< ResponsesType > * _medium ) const;

	common::CMediumFilter< ResponsesType > * getMediumFilter() const;

	std::vector< unsigned char > getSigned() const;

	CPubKey getKey() const;

	std::vector< unsigned char > getPayload()const;

	uint256 getActionKey() const;
private:
	common::CMediumFilter< ResponsesType > * m_mediumFilter;

	std::vector< unsigned char > m_signed;

	CPubKey m_key;

	std::vector< unsigned char > m_payload;

	uint256 const m_actionKey;
};

template < class ResponsesType >
CIdentifyResponse< ResponsesType >::CIdentifyResponse( unsigned int _shortPtr, std::vector< unsigned char > const & _signed, CPubKey const & _key, std::vector< unsigned char > const & _payload, uint256 const & _actionKey )
	: m_mediumFilter( new common::CMediumFilter< ResponsesType >( -1, -1, new CAcceptFilterByShortPtr< ResponsesType >( _shortPtr ) ) )// this may be not  right
	, m_signed( _signed )
	, m_key( _key )
	, m_payload( _payload )
	, m_actionKey( _actionKey )
{
}

template < class ResponsesType >
void
CIdentifyResponse< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
common::CMediumFilter< ResponsesType > *
CIdentifyResponse< ResponsesType >::getMediumFilter() const
{
	return m_mediumFilter;
}

template < class ResponsesType >
std::vector< unsigned char >
CIdentifyResponse< ResponsesType >::getSigned() const
{
	return m_signed;
}

template < class ResponsesType >
CPubKey
CIdentifyResponse< ResponsesType >::getKey() const
{
	return m_key;
}

template < class ResponsesType >
std::vector< unsigned char >
CIdentifyResponse< ResponsesType >::getPayload()const
{
	return m_payload;
}

template < class ResponsesType >
uint256
CIdentifyResponse< ResponsesType >::getActionKey() const
{
	return m_actionKey;
}


template < class ResponsesType >
class CContinueReqest : public common::CRequest< ResponsesType >
{
public:
	CContinueReqest( uint256 const & _id, common::CMediumFilter< ResponsesType > * _mediumFilter );

	void accept( common::CMedium< ResponsesType > * _medium ) const;

	common::CMediumFilter< ResponsesType > * getMediumFilter() const;

	uint256 getRequestId()const;
private:

	uint256 const m_id;
	common::CMediumFilter< ResponsesType > * m_mediumFilter;
};

template < class ResponsesType >
CContinueReqest< ResponsesType >::CContinueReqest( uint256 const & _id, common::CMediumFilter< ResponsesType > * _mediumFilter )
	: m_id( _id )
	, m_mediumFilter( _mediumFilter )
{
}

template < class ResponsesType >
void
CContinueReqest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
common::CMediumFilter< ResponsesType > *
CContinueReqest< ResponsesType >::getMediumFilter() const
{
	return m_mediumFilter;
}

template < class ResponsesType >
uint256
CContinueReqest< ResponsesType >::getRequestId() const
{
	return m_id;
}

template < class ResponsesType >
class CConnectToNodeRequest : public common::CRequest< ResponsesType >
{
public:
	CConnectToNodeRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress, int _kind );

	virtual void accept( common::CMedium< ResponsesType > * _medium ) const;

	virtual int getMediumFilter() const;

	std::string getAddress() const;

	CAddress getServiceAddress() const;
private:
	std::string const m_trackerAddress;

	CAddress const m_serviceAddress;

	int m_kind;

	uint256 const m_actionKey;
};

template < class ResponsesType >
CConnectToNodeRequest< ResponsesType >::CConnectToNodeRequest( std::string const & _trackerAddress, CAddress const & _serviceAddress, int _kind  )
	:m_trackerAddress( _trackerAddress )
	,m_serviceAddress( _serviceAddress )
	,m_kind( _kind )
{
}

template < class ResponsesType >
void
CConnectToNodeRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
int
CConnectToNodeRequest< ResponsesType >::getMediumFilter() const
{
	return m_kind;
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

	common::CMediumFilter< ResponsesType > * m_mediumFilter;
};

template < class ResponsesType >
CNetworkRoleRequest< ResponsesType >::CNetworkRoleRequest( uint256 const & _actionKey, int _role, common::CMediumFilter< ResponsesType > * _mediumFilter )
	: m_actionKey( _actionKey )
	, m_role( _role )
	, m_mediumFilter( _mediumFilter )
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
	return m_mediumFilter;
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
	CKnownNetworkInfoRequest( uint256 const & _actionKey, std::vector< CValidNodeInfo > const & _networkInfo, int _filterClass );

	~CKnownNetworkInfoRequest();

	virtual void accept( common::CMedium< ResponsesType > * _medium ) const;

	virtual common::CMediumFilter< ResponsesType > * getMediumFilter() const;

	std::vector< CValidNodeInfo > getNetworkInfo() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;

	std::vector< CValidNodeInfo > m_networkInfo;

	common::CMediumFilter< ResponsesType > * m_mediumFilter;
};

template < class ResponsesType >
CKnownNetworkInfoRequest< ResponsesType >::CKnownNetworkInfoRequest( uint256 const & _actionKey, std::vector< CValidNodeInfo > const & _networkInfo, int _filterClass )
	: m_actionKey( _actionKey )
	, m_networkInfo( _networkInfo )
	, m_mediumFilter( new common::CMediumFilter< ResponsesType >( _filterClass ) )
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
	return m_mediumFilter;
}

template < class ResponsesType >
std::vector< CValidNodeInfo >
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
CKnownNetworkInfoRequest< ResponsesType >::~CKnownNetworkInfoRequest()
{
	delete m_mediumFilter;
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

	std::vector< CValidNodeInfo > m_networkInfo;

	common::CMediumFilter< ResponsesType > * m_mediumFilter;
};

template < class ResponsesType >
CAckRequest< ResponsesType >::CAckRequest( uint256 const & _actionKey, common::CMediumFilter< ResponsesType > * _mediumFilter )
	: m_actionKey( _actionKey )
	, m_mediumFilter( _mediumFilter )
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
	return m_mediumFilter;
}

template < class ResponsesType >
uint256
CAckRequest< ResponsesType >::getActionKey() const
{
	return m_actionKey;
}



}

#endif // MEDIUM_REQUESTS_H
