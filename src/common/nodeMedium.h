// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_MEDIUM_H
#define NODE_MEDIUM_H

#include "common/medium.h"
#include "common/communicationProtocol.h"
#include <boost/variant.hpp>
#include "common/medium.h"
#include "common/mediumRequests.h"
#include "common/nodeMedium.h"
#include "common/actionHandler.h"
#include "common/nodesManager.h"
#include "common/authenticationProvider.h"

namespace common
{

typedef boost::variant< common::CIdentifyMessage > ProtocolMessage;

template < class RequestType > class CIdentifyRequest;

template < class ResponseType >
class CNodeMedium : public common::CMedium< ResponseType >
{
public:
	CNodeMedium( common::CSelfNode * _selfNode ):m_usedNode( _selfNode ){};

	bool serviced() const;

	bool flush();

	bool getResponse( std::vector< ResponseType > & _requestResponse ) const;

	void clearResponses();

	void add( common::CRequest< ResponseType > const * _request );

	void add( CIdentifyRequest< ResponseType > const * _request );

	void add( CIdentifyResponse< ResponseType > const * _request );

	void add( CContinueReqest< ResponseType > const * _request );

	void add( CNetworkRoleRequest< ResponseType > const * _request );

	void add( CAckRequest< ResponseType > const * _request );

	void setResponse( uint256 const & _id, ResponseType const & _responses );

	common::CSelfNode * getNode() const;

protected:
	common::CSelfNode * m_usedNode;

	mutable boost::mutex m_mutex;
	std::map< uint256, ResponseType > m_responses;

	static uint256 m_counter;

	std::vector< common::CMessage > m_messages;

	std::vector< uint256 > m_indexes;

};

template < class ResponseType >
bool
CNodeMedium< ResponseType >::serviced() const
{
	return true;
}

template < class ResponseType >
bool
CNodeMedium< ResponseType >::flush()
{
	BOOST_FOREACH( common::CMessage const & message ,m_messages )
	{
		m_usedNode->setMessageToSend( message );
	}
	m_messages.clear();
	return true;

}
// it piss me off that I have to  keep this  bitch   here
extern std::vector< uint256 > deleteList;

template < class ResponseType >
bool
CNodeMedium< ResponseType >::getResponse( std::vector< ResponseType > & _requestResponse ) const
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	BOOST_FOREACH( uint256 const & id, m_indexes )
	{
		typename std::map< uint256, ResponseType >::const_iterator iterator = m_responses.find( id );
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

template < class ResponseType >
void
CNodeMedium< ResponseType >::clearResponses()
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	BOOST_FOREACH( uint256 const & id, deleteList )
	{
		m_responses.erase( id );
	}
	deleteList.clear();
	m_indexes.clear();
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::setResponse( uint256 const & _id, ResponseType const & _response )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_responses.insert( std::make_pair( _id, _response ) );
}

template < class ResponseType >
common::CSelfNode *
CNodeMedium< ResponseType >::getNode() const
{
	return m_usedNode;
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( common::CRequest< ResponseType > const * _request )
{
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CIdentifyRequest< ResponseType > const * _request )
{
	common::CIdentifyMessage identifyMessage;

	identifyMessage.m_payload = _request->getPayload();

	identifyMessage.m_actionKey = _request->getActionKey();

	common::CMessage message( identifyMessage );

	m_messages.push_back( message );

	uint256 hash = Hash( &identifyMessage.m_payload.front(), &identifyMessage.m_payload.back() );
	m_indexes.push_back( hash );
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CIdentifyResponse< ResponseType > const * _request )
{
	common::CIdentifyMessage identifyMessage;

	identifyMessage.m_payload = _request->getPayload();

	identifyMessage.m_signed = _request->getSigned();

	identifyMessage.m_key = _request->getKey();

	identifyMessage.m_actionKey = _request->getActionKey();

	common::CMessage message( identifyMessage );

	m_messages.push_back( message );

	uint256 hash = Hash( &identifyMessage.m_payload.front(), &identifyMessage.m_payload.back() );
	m_indexes.push_back( hash );
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CContinueReqest< ResponseType > const * _request )
{
	m_indexes.push_back( _request->getRequestId() );
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CNetworkRoleRequest< ResponseType > const * _request )
{
	common::CNetworkRole networkRole;

	networkRole.m_actionKey = _request->getActionKey();

	networkRole.m_role = _request->getRole();

	common::CMessage message( networkRole );

	common::CMessage orginalMessage;

	common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), common::CAuthenticationProvider::getInstance()->getMyKey() );

	m_messages.push_back( message );

	m_indexes.push_back( _request->getActionKey() );
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CAckRequest< ResponseType > const * _request )
{
	common::CAck ack;

	ack.m_actionKey = _request->getActionKey();

	common::CMessage message( ack );

	common::CMessage orginalMessage;

	common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), common::CAuthenticationProvider::getInstance()->getMyKey() );

	m_messages.push_back( message );

	m_indexes.push_back( _request->getActionKey() );
}

}

#endif // NODE_MEDIUM_H
