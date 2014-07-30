// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MEDIUM_REQUESTS_H
#define MEDIUM_REQUESTS_H

#include "common/request.h"
#include "net.h"
#include "common/medium.h"

#include "util.h"

namespace common
{

template < class ResponsesType >
class CIdentifyRequest : public common::CRequest< ResponsesType >
{
public:
	CIdentifyRequest( unsigned int _kind, std::vector< unsigned char > const & _payload, uint256 const & _actionKey );

	void accept( common::CMedium< ResponsesType > * _medium ) const;

	int getKind() const;

	std::vector< unsigned char > getPayload() const;

	uint256 getActionKey() const;
private:

	std::vector< unsigned char > m_payload;

	unsigned int m_kind;

	uint256 const m_actionKey;
};

template < class ResponsesType >
CIdentifyRequest< ResponsesType >::CIdentifyRequest( unsigned int _kind, std::vector< unsigned char > const & _payload, uint256 const & _actionKey )
	: m_kind( _kind )
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
void
CIdentifyRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
int
CIdentifyRequest< ResponsesType >::getKind() const
{
	return m_kind;
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

	int getKind() const;


	std::vector< unsigned char > getSigned() const;

	CPubKey getKey() const;

	std::vector< unsigned char > getPayload()const;

	uint256 getActionKey() const;
private:
	unsigned int m_kind;
	std::vector< unsigned char > m_signed;
	CPubKey m_key;
	std::vector< unsigned char > m_payload;

	uint256 const m_actionKey;
};

template < class ResponsesType >
CIdentifyResponse< ResponsesType >::CIdentifyResponse( unsigned int _kind, std::vector< unsigned char > const & _signed, CPubKey const & _key, std::vector< unsigned char > const & _payload, uint256 const & _actionKey )
	: m_kind( _kind )
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
int
CIdentifyResponse< ResponsesType >::getKind() const
{
	return m_kind;
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
	CContinueReqest( uint256 const & _id, unsigned int _kind );

	void accept( common::CMedium< ResponsesType > * _medium ) const;

	int getKind() const;

	uint256 getRequestId()const;
private:

	uint256 const m_id;
	unsigned int m_kind;
};

template < class ResponsesType >
CContinueReqest< ResponsesType >::CContinueReqest( uint256 const & _id, unsigned int _kind )
	: m_id( _id )
	, m_kind( _kind )
{
}

template < class ResponsesType >
void
CContinueReqest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
int
CContinueReqest< ResponsesType >::getKind() const
{
	return m_kind;
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

	virtual int getKind() const;

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
CConnectToNodeRequest< ResponsesType >::getKind() const
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
	CNetworkRoleRequest( uint256 const & _actionKey, int _role, int _kind );

	virtual void accept( common::CMedium< ResponsesType > * _medium ) const;

	virtual int getKind() const;

	int getRole() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;

	int m_role;

	int m_kind;
};

template < class ResponsesType >
CNetworkRoleRequest< ResponsesType >::CNetworkRoleRequest( uint256 const & _actionKey, int _role, int _kind )
	: m_actionKey( _actionKey )
	, m_role( _role )
	, m_kind( _kind )
{
}

template < class ResponsesType >
void
CNetworkRoleRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
int
CNetworkRoleRequest< ResponsesType >::getKind() const
{
	return m_kind;
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
	CKnownNetworkInfoRequest( uint256 const & _actionKey, std::vector< CValidNodeInfo > const & _networkInfo, int _kind );

	virtual void accept( common::CMedium< ResponsesType > * _medium ) const;

	virtual int getKind() const;

	std::vector< CValidNodeInfo > getNetworkInfo() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;

	std::vector< CValidNodeInfo > m_networkInfo;

	int m_kind;
};

template < class ResponsesType >
CKnownNetworkInfoRequest< ResponsesType >::CKnownNetworkInfoRequest( uint256 const & _actionKey, std::vector< CValidNodeInfo > const & _networkInfo, int _kind )
	: m_actionKey( _actionKey )
	, m_networkInfo( _networkInfo )
	, m_kind( _kind )
{
}

template < class ResponsesType >
void
CKnownNetworkInfoRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
int
CKnownNetworkInfoRequest< ResponsesType >::getKind() const
{
	return m_kind;
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
class CAckRequest : public common::CRequest< ResponsesType >
{
public:
	CAckRequest( uint256 const & _actionKey, int _kind );

	virtual void accept( common::CMedium< ResponsesType > * _medium ) const;

	virtual int getKind() const;

	uint256 getActionKey() const;
private:
	uint256 const m_actionKey;

	std::vector< CValidNodeInfo > m_networkInfo;

	int m_kind;
};

template < class ResponsesType >
CAckRequest< ResponsesType >::CAckRequest( uint256 const & _actionKey, int _kind )
	: m_actionKey( _actionKey )
	, m_kind( _kind )
{
}

template < class ResponsesType >
void
CAckRequest< ResponsesType >::accept( common::CMedium< ResponsesType > * _medium ) const
{
	_medium->add( this );
}

template < class ResponsesType >
int
CAckRequest< ResponsesType >::getKind() const
{
	return m_kind;
}

template < class ResponsesType >
uint256
CAckRequest< ResponsesType >::getActionKey() const
{
	return m_actionKey;
}



}

#endif // MEDIUM_REQUESTS_H
