// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "identifyRequest.h"


namespace  tracker
{

CIdentifyRequest::CIdentifyRequest( unsigned int _kind, std::vector< unsigned char > const & _payload )
	: m_kind( _kind )
	, m_payload( _payload )
{
}

void
CIdentifyRequest::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CIdentifyRequest::getKind() const
{
	return m_kind;
}

std::vector< unsigned char >
CIdentifyRequest::getPayload() const
{
	return m_payload;
}

CSelfNode *
CIdentifyRequest::getNode() const
{
	return m_node;
}

CIdentifyResponse::CIdentifyResponse( unsigned int _kind, std::vector< unsigned char > const & _signed, uint160 _keyId )
	: m_kind( _kind )
	, m_signed( _signed )
	, m_keyId( _keyId )
{
}

void
CIdentifyResponse::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CIdentifyResponse::getKind() const
{
	return m_kind;
}

CSelfNode *
CIdentifyResponse::getNode() const
{
	return m_node;
}

std::vector< unsigned char >
CIdentifyResponse::getSigned() const
{
	return m_signed;
}

uint160
CIdentifyResponse::getKeyID() const
{
	return m_keyId;
}

}
