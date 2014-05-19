// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "identifyRequest.h"
#include "common/medium.h"
#include "util.h"
#include "trackerMediumsKinds.h"

namespace  tracker
{

CIdentifyRequest::CIdentifyRequest( CNode * _node )
{
	for ( unsigned int i = 0; i < ms_randomPayloadLenght; i++ )
	{
		m_payload[ i ] = insecure_rand() % 256;
	}
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

}
