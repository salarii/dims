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

	void setResponse( uint256 const & _id, ResponseType const & _responses );

	bool isIdentifyMessageKnown( uint256 const & _payloadHash ) const;

	common::CSelfNode * getNode() const;
private:
	common::CSelfNode * m_usedNode;

	mutable boost::mutex m_mutex;
	std::map< uint256, ResponseType > m_responses;

	std::set< uint256 > m_findIdentifyMessage;

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
	BOOST_FOREACH( uint256 const & id, deleteList )
	{
		m_responses.erase( id );
		m_findIdentifyMessage.erase( id );// not  correct because there are  scenarios  when this  will not work
	}
	deleteList.clear();
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::setResponse( uint256 const & _id, ResponseType const & _response )
{
	m_responses.insert( std::make_pair( _id, _response ) );
}

template < class ResponseType >
bool
CNodeMedium< ResponseType >::isIdentifyMessageKnown( uint256 const & _payloadHash ) const
{
	std::set< uint256 >::const_iterator iterator = m_findIdentifyMessage.find( _payloadHash );

	return iterator != m_findIdentifyMessage.end();
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

	common::CMessage message( identifyMessage );

	m_messages.push_back( message );

	uint256 hash = Hash( &identifyMessage.m_payload.front(), &identifyMessage.m_payload.back() );
	m_indexes.push_back( hash );
	m_findIdentifyMessage.insert( hash );
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CIdentifyResponse< ResponseType > const * _request )
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

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CContinueReqest< ResponseType > const * _request )
{
	m_indexes.push_back( _request->getRequestId() );

}

}

#endif // NODE_MEDIUM_H
