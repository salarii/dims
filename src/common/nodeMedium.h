// Copyright (c) 2014-2015 Dims dev-team
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

	bool getResponseAndClear( std::multimap< CRequest< ResponseType >const*, ResponseType > & _requestResponse );

	void add( common::CRequest< ResponseType >const * _request );

	void add( CIdentifyRequest< ResponseType > const * _request );

	void add( CIdentifyResponse< ResponseType > const * _request );

	void add( CNetworkRoleRequest< ResponseType > const * _request );

	void add( CKnownNetworkInfoRequest< ResponseType > const * _request );

	void add( CAckRequest< ResponseType > const * _request );

	void add( CEndRequest< ResponseType > const * _request );

	void add( CResultRequest< ResponseType > const * _request );

	void setResponse( uint256 const & _id, ResponseType const & _responses );

	common::CSelfNode * getNode() const;

protected:
	void clearResponses();

	void updateLastRequest( uint256 const & _id, common::CRequest< ResponseType >const* _request );
protected:
	common::CSelfNode * m_usedNode;

	mutable boost::mutex m_mutex;
	std::multimap< uint256, ResponseType > m_responses;

	static uint256 m_counter;

	std::vector< common::CMessage > m_messages;

	std::vector< uint256 > m_indexes;

	std::map< uint256, common::CRequest< ResponseType >const* > m_lastRequestForAction;
};

template < class ResponseType >
bool
CNodeMedium< ResponseType >::serviced() const
{
	return true;
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::updateLastRequest( uint256 const & _id, common::CRequest< ResponseType >const* _request )
{
	if ( m_lastRequestForAction.find( _id ) != m_lastRequestForAction.end() )
		m_lastRequestForAction.erase( _id );

	m_lastRequestForAction.insert( std::make_pair( _id, _request ) );
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

extern std::vector< uint256 > deleteList;

template < class ResponseType >
bool
CNodeMedium< ResponseType >::getResponseAndClear( std::multimap< CRequest< ResponseType >const*, ResponseType > & _requestResponse )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	BOOST_FOREACH( uint256 const & id, m_indexes )
	{
		typename std::multimap< uint256, ResponseType >::const_iterator iterator = m_responses.lower_bound( id );
		if ( iterator != m_responses.upper_bound( id ) )
		{
			_requestResponse.insert( std::make_pair( m_lastRequestForAction.find( id )->second, iterator->second ) );
			deleteList.push_back( id );
		}
	}
	clearResponses();
	return true;
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::clearResponses()
{
	BOOST_FOREACH( uint256 const & id, deleteList )
	{
		m_responses.erase( m_responses.lower_bound( id ) );
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

	common::CMessage message( identifyMessage, _request->getActionKey() );

	m_messages.push_back( message );

	m_indexes.push_back( _request->getActionKey() );

	updateLastRequest( _request->getActionKey(), (common::CRequest< ResponseType >const*)_request );
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CIdentifyResponse< ResponseType > const * _request )
{
	common::CIdentifyMessage identifyMessage;

	identifyMessage.m_payload = _request->getPayload();

	identifyMessage.m_signed = _request->getSigned();

	identifyMessage.m_key = _request->getKey();

	common::CMessage message( identifyMessage, _request->getActionKey() );

	m_messages.push_back( message );

	m_indexes.push_back( _request->getActionKey() );

	updateLastRequest( _request->getActionKey(), (common::CRequest< ResponseType >const*)_request );
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CNetworkRoleRequest< ResponseType > const * _request )
{
	common::CNetworkRole networkRole;

	networkRole.m_role = _request->getRole();

	common::CMessage message( networkRole, _request->getActionKey() );

	m_messages.push_back( message );

	m_indexes.push_back( _request->getActionKey() );

	updateLastRequest( _request->getActionKey(), (common::CRequest< ResponseType >const*)_request );
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CKnownNetworkInfoRequest< ResponseType > const * _request )
{

	common::CMessage message( _request->getNetworkInfo(), _request->getActionKey() );

	m_messages.push_back( message );

	m_indexes.push_back( _request->getActionKey() );

		updateLastRequest( _request->getActionKey(), (common::CRequest< ResponseType >const*)_request );
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CAckRequest< ResponseType > const * _request )
{
	common::CAck ack;

	common::CMessage message( ack, _request->getActionKey() );

	m_messages.push_back( message );

	m_indexes.push_back( _request->getActionKey() );

	m_responses.insert( std::make_pair( _request->getActionKey(), common::CAckPromptResult() ) );

		updateLastRequest( _request->getActionKey(), (common::CRequest< ResponseType >const*)_request );
}


template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CEndRequest< ResponseType > const * _request )
{
	common::CEnd end;

	common::CMessage message( end, _request->getActionKey() );

	m_messages.push_back( message );

	m_indexes.push_back( _request->getActionKey() );

	updateLastRequest( _request->getActionKey(), (common::CRequest< ResponseType >const*)_request );//most likely wrong, but handy for time being
}

template < class ResponseType >
void
CNodeMedium< ResponseType >::add( CResultRequest< ResponseType > const * _request )
{
	common::CResult result;

	common::CMessage message( result, _request->getActionKey() );

	m_messages.push_back( message );

	m_indexes.push_back( _request->getActionKey() );

	updateLastRequest( _request->getActionKey(), (common::CRequest< ResponseType >const*)_request );
}

}

#endif // NODE_MEDIUM_H
