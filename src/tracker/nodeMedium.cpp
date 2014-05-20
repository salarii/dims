// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/medium.h"
#include "identifyRequest.h"
#include "communicationProtocol.h"
#include "nodeMedium.h"

namespace tracker
{

bool
CNodeMedium::serviced() const
{

}


bool
CNodeMedium::flush()
{

}

bool
CNodeMedium::getResponse( std::vector< TrackerResponses > & _requestResponse ) const
{

}


void
CNodeMedium::add( common::CRequest< TrackerResponses > const * _request )
{

}

void
CNodeMedium::add( CIdentifyRequest const * _request )
{
	CIdentifyMessage identifyMessage;
	identifyMessage.m_payload = _request->getPayload();

	CMessage message( identifyMessage );

	//_request->

}

}
