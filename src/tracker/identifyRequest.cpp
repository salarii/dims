// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "identifyRequest.h"


namespace  tracker
{

CIdentifyRequest::CIdentifyRequest( CNode * _node, std::vector< unsigned char > const & _payload )
	: m_node( _node )
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
	return CTrackerMediumsKinds::Nodes;// fix  it
}

std::vector< unsigned char >
CIdentifyRequest::getPayload() const
{
	return m_payload;
}

CIdentifyResponse::CIdentifyResponse( CNode * _node, std::vector< unsigned char > const & _signed, uint160 _keyId )
{}

void
CIdentifyResponse::accept( common::CMedium< TrackerResponses > * _medium ) const
{
	_medium->add( this );
}

int
CIdentifyResponse::getKind() const
{
	return 0;
}

CNode *
CIdentifyResponse::getNode() const
{
	return 0;
}

}
