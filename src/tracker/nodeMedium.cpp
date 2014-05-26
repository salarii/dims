// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/medium.h"
#include "identifyRequest.h"
#include "nodeMedium.h"
#include "continueRequest.h"
#include "common/actionHandler.h"
#include "nodesManager.h"
#include "connectTrackerAction.h"

namespace tracker
{

uint256 CNodeMedium::m_counter = 0;

bool
CNodeMedium::serviced() const
{
	return true;
}


bool
CNodeMedium::flush()
{
	BOOST_FOREACH( common::CMessage const & message ,m_messages )
	{
		m_usedNode->setMessageToSend( message );
	}
	m_messages.clear();
	return true;

}
// it piss me off that I have to  keep this  bitch   here
std::vector< uint256 > deleteList;

bool
CNodeMedium::getResponse( std::vector< TrackerResponses > & _requestResponse ) const
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	BOOST_FOREACH( uint256 const & id, m_indexes )
	{
		std::map< uint256, TrackerResponses >::const_iterator iterator = m_responses.find( id );
		if ( iterator != m_responses.end() )
		{
			_requestResponse.push_back( iterator->second );
			deleteList.push_back( id );
		}
		else
		{
			_requestResponse.push_back( CContinueResult( id ) );
		}
	}

	return true;
}

void
CNodeMedium::clearResponses()
{
	BOOST_FOREACH( uint256 const & id, deleteList )
	{
		m_responses.erase( id );
		m_findIdentifyMessage.erase( id );// not  correct because there are  scenarios  when this  will not work
	}
	deleteList.clear();
}

void
CNodeMedium::setResponse( uint256 const & _id, TrackerResponses const & _responses )
{
	m_responses.insert( std::make_pair( _id, _responses ) );
}

bool
CNodeMedium::isIdentifyMessageKnown( uint256 const & _payloadHash ) const
{
	std::set< uint256 >::const_iterator iterator = m_findIdentifyMessage.find( _payloadHash );

	return iterator != m_findIdentifyMessage.end();
}

common::CSelfNode *
CNodeMedium::getNode() const
{
	return m_usedNode;
}

void
CNodeMedium::add( common::CRequest< TrackerResponses > const * _request )
{
}

void
CNodeMedium::add( CIdentifyRequest const * _request )
{
	common::CIdentifyMessage identifyMessage;
	identifyMessage.m_payload = _request->getPayload();

	common::CMessage message( identifyMessage );

	m_messages.push_back( message );

	uint256 hash = Hash( &identifyMessage.m_payload.front(), &identifyMessage.m_payload.back() );
	m_indexes.push_back( hash );
	m_findIdentifyMessage.insert( hash );
}

void
CNodeMedium::add( CIdentifyResponse const * _request )
{
	common::CIdentifyMessage identifyMessage;

	identifyMessage.m_payload = _request->getPayload();

	identifyMessage.m_signed = _request->getSigned();

	identifyMessage.m_key = _request->getKeyID();

	common::CMessage message( identifyMessage );

	m_messages.push_back( message );

	uint256 hash = Hash( &identifyMessage.m_payload.front(), &identifyMessage.m_payload.back() );
	m_indexes.push_back( hash );
	m_findIdentifyMessage.insert( hash );
}


void
CNodeMedium::add( CContinueReqest const * _request )
{
	m_indexes.push_back( _request->getRequestId() );

}

}
