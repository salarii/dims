// Copyright (c) 2014 Ratcoin dev-team
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

template < class RequestType >
class CIdentifyRequest : public common::CRequest< RequestType >
{
public:
	CIdentifyRequest( unsigned int _kind, std::vector< unsigned char > const & _payload );

	void accept( common::CMedium< RequestType > * _medium ) const;

	int getKind() const;

	std::vector< unsigned char > getPayload() const;
private:

	std::vector< unsigned char > m_payload;

	unsigned int m_kind;
};

template < class RequestType >
CIdentifyRequest< RequestType >::CIdentifyRequest( unsigned int _kind, std::vector< unsigned char > const & _payload )
	: m_kind( _kind )
	, m_payload( _payload )
{
}

template < class RequestType >
void
CIdentifyRequest< RequestType >::accept( common::CMedium< RequestType > * _medium ) const
{
	_medium->add( this );
}

template < class RequestType >
int
CIdentifyRequest< RequestType >::getKind() const
{
	return m_kind;
}

template < class RequestType >
std::vector< unsigned char >
CIdentifyRequest< RequestType >::getPayload() const
{
	return m_payload;
}


template < class RequestType >
class CIdentifyResponse : public common::CRequest< RequestType >
{
public:
	CIdentifyResponse( unsigned int _kind, std::vector< unsigned char > const & _signed, uint160 _keyId, std::vector< unsigned char > const & _payload );

	void accept( common::CMedium< RequestType > * _medium ) const;

	int getKind() const;


	std::vector< unsigned char > getSigned() const;

	uint160 getKeyID() const;

	std::vector< unsigned char > getPayload()const;
private:
	unsigned int m_kind;
	std::vector< unsigned char > m_signed;
	uint160 m_keyId;
	std::vector< unsigned char > m_payload;
};

template < class RequestType >
CIdentifyResponse< RequestType >::CIdentifyResponse( unsigned int _kind, std::vector< unsigned char > const & _signed, uint160 _keyId, std::vector< unsigned char > const & _payload )
	: m_kind( _kind )
	, m_signed( _signed )
	, m_keyId( _keyId )
	, m_payload( _payload )
{
}

template < class RequestType >
void
CIdentifyResponse< RequestType >::accept( common::CMedium< RequestType > * _medium ) const
{
	_medium->add( this );
}

template < class RequestType >
int
CIdentifyResponse< RequestType >::getKind() const
{
	return m_kind;
}

template < class RequestType >
std::vector< unsigned char >
CIdentifyResponse< RequestType >::getSigned() const
{
	return m_signed;
}

template < class RequestType >
uint160
CIdentifyResponse< RequestType >::getKeyID() const
{
	return m_keyId;
}

template < class RequestType >
std::vector< unsigned char >
CIdentifyResponse< RequestType >::getPayload()const
{
	return m_payload;
}

template < class RequestType >
class CContinueReqest : public common::CRequest< RequestType >
{
public:
	CContinueReqest( uint256 const & _id, unsigned int _kind );

	void accept( common::CMedium< RequestType > * _medium ) const;

	int getKind() const;

	uint256 getRequestId()const;
private:

	uint256 const m_id;
	unsigned int m_kind;
};

template < class RequestType >
CContinueReqest< RequestType >::CContinueReqest( uint256 const & _id, unsigned int _kind )
	: m_id( _id )
	, m_kind( _kind )
{
}

template < class RequestType >
void
CContinueReqest< RequestType >::accept( common::CMedium< RequestType > * _medium ) const
{
	_medium->add( this );
}

template < class RequestType >
int
CContinueReqest< RequestType >::getKind() const
{
	return m_kind;
}

template < class RequestType >
uint256
CContinueReqest< RequestType >::getRequestId() const
{
	return m_id;
}


}

#endif // MEDIUM_REQUESTS_H
