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
	CIdentifyResponse( unsigned int _kind, std::vector< unsigned char > const & _signed, uint160 _keyId, std::vector< unsigned char > const & _payload, uint256 const & _actionKey );

	void accept( common::CMedium< ResponsesType > * _medium ) const;

	int getKind() const;


	std::vector< unsigned char > getSigned() const;

	uint160 getKeyID() const;

	std::vector< unsigned char > getPayload()const;

	uint256 getActionKey() const;
private:
	unsigned int m_kind;
	std::vector< unsigned char > m_signed;
	uint160 m_keyId;
	std::vector< unsigned char > m_payload;

	uint256 const m_actionKey;
};

template < class ResponsesType >
CIdentifyResponse< ResponsesType >::CIdentifyResponse( unsigned int _kind, std::vector< unsigned char > const & _signed, uint160 _keyId, std::vector< unsigned char > const & _payload, uint256 const & _actionKey )
	: m_kind( _kind )
	, m_signed( _signed )
	, m_keyId( _keyId )
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
uint160
CIdentifyResponse< ResponsesType >::getKeyID() const
{
	return m_keyId;
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

}

#endif // MEDIUM_REQUESTS_H
