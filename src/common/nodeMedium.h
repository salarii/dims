// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_MEDIUM_H
#define NODE_MEDIUM_H

#include <boost/variant.hpp>

#include "common/medium.h"
#include "common/communicationProtocol.h"
#include "common/mediumRequests.h"
#include "common/authenticationProvider.h"
#include "common/selfNode.h"
// fix  this !!!
namespace common
{

typedef boost::variant< common::CIdentifyMessage > ProtocolMessage;

template < class _Medium >
class CNodeMedium : public _Medium
{
public:
	typedef TYPE(_Medium) Type;
	typedef RESPONSE_TYPE(_Medium) Response;
public:
	CNodeMedium( common::CSelfNode * _selfNode ):m_usedNode( _selfNode ){};

	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::multimap< CRequest< Type >const*, Response > & _requestResponse );

	void add( common::CRequest< Type >const * _request );

	void add( CSendIdentifyDataRequest< Type > const * _request );

	void add( CNetworkRoleRequest< Type > const * _request );

	void add( CKnownNetworkInfoRequest< Type > const * _request );

	void add( CAckRequest< Type > const * _request );

	void add( CEndRequest< Type > const * _request );

	void add( CResultRequest< Type > const * _request );

	void add( CPingRequest< Type > const * _request );

	void add( CPongRequest< Type > const * _request );

	void setResponse( uint256 const & _id, Response const & _responses );

	common::CSelfNode * getNode() const;

protected:
	void clearResponses();
// this  is wrong, but for now let it be
	void updateLastRequest( uint256 const & _id, common::CRequest< Type >const* _request );
protected:
	common::CSelfNode * m_usedNode;

	mutable boost::mutex m_mutex;
	std::multimap< uint256, Response > m_responses;

	static uint256 m_counter;

	std::vector< common::CMessage > m_messages;

	std::set< uint256 > m_indexes;

	std::map< uint256, common::CRequest< Type >const* > m_lastRequestForAction;
};

template < class _Medium >
bool
CNodeMedium< _Medium >::serviced() const
{
	return !m_responses.empty();
}

template < class _Medium >
void
CNodeMedium< _Medium >::updateLastRequest( uint256 const & _id, common::CRequest< Type >const* _request )
{
	if ( m_lastRequestForAction.find( _id ) != m_lastRequestForAction.end() )
		m_lastRequestForAction.erase( _id );

	m_lastRequestForAction.insert( std::make_pair( _id, _request ) );
}

template < class _Medium >
bool
CNodeMedium< _Medium >::flush()
{
	BOOST_FOREACH( common::CMessage const & message ,m_messages )
	{
		m_usedNode->setMessageToSend( message );
	}
	m_messages.clear();
	return true;

}

extern std::vector< uint256 > deleteList;

template < class _Medium >
bool
CNodeMedium< _Medium >::getResponseAndClear( std::multimap< CRequest< Type >const*, RESPONSE_TYPE(_Medium) > & _requestResponse )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	BOOST_FOREACH( uint256 const & id, m_indexes )
	{
		typename std::multimap< uint256, Response >::const_iterator iterator = m_responses.lower_bound( id );
		while ( iterator != m_responses.upper_bound( id ) )
		{
			_requestResponse.insert( std::make_pair( m_lastRequestForAction.find( id )->second, iterator->second ) );
			deleteList.push_back( id );
			++iterator;
		}
	}
	clearResponses();
	return true;
}

template < class _Medium >
void
CNodeMedium< _Medium >::clearResponses()
{
	BOOST_FOREACH( uint256 const & id, deleteList )
	{
		m_responses.erase( m_responses.lower_bound( id ) );
	}
	deleteList.clear();
	m_indexes.clear();
}

template < class _Medium >
void
CNodeMedium< _Medium >::setResponse( uint256 const & _id, Response const & _response )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_responses.insert( std::make_pair( _id, _response ) );
	m_indexes.insert( _id );
}

template < class _Medium >
common::CSelfNode *
CNodeMedium< _Medium >::getNode() const
{
	return m_usedNode;
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( common::CRequest< Type > const * _request )
{
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CSendIdentifyDataRequest< Type > const * _request )
{
	common::CIdentifyMessage identifyMessage;

	identifyMessage.m_payload = _request->getPayload();

	identifyMessage.m_signed = _request->getSigned();

	identifyMessage.m_key = _request->getKey();

	common::CMessage message( identifyMessage, _request->getActionKey() );

	m_messages.push_back( message );

	updateLastRequest( _request->getActionKey(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CNetworkRoleRequest< Type > const * _request )
{
	common::CNetworkRole networkRole;

	networkRole.m_role = _request->getRole();

	common::CMessage message( networkRole, _request->getActionKey() );

	m_messages.push_back( message );

	updateLastRequest( _request->getActionKey(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CKnownNetworkInfoRequest< Type > const * _request )
{

	common::CMessage message( _request->getNetworkInfo(), _request->getActionKey() );

	m_messages.push_back( message );

		updateLastRequest( _request->getActionKey(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CAckRequest< Type > const * _request )
{
	common::CAck ack;

	common::CMessage message( ack, _request->getActionKey() );

	m_messages.push_back( message );

		updateLastRequest( _request->getActionKey(), (common::CRequest< Type >const*)_request );
}


template < class _Medium >
void
CNodeMedium< _Medium >::add( CEndRequest< Type > const * _request )
{
	common::CEnd end;

	common::CMessage message( end, _request->getActionKey() );

	m_messages.push_back( message );

	updateLastRequest( _request->getActionKey(), (common::CRequest< Type >const*)_request );//most likely wrong, but handy for time being
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CResultRequest< Type > const * _request )
{
	common::CResult result;

	result.m_result = _request->getResult();

	common::CMessage message( result, _request->getActionKey() );

	m_messages.push_back( message );

	updateLastRequest( _request->getActionKey(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CPingRequest< Type > const * _request )
{
	CPing ping;

	common::CMessage message( ping, _request->getActionKey() );

	m_messages.push_back( message );

		updateLastRequest( _request->getActionKey(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CPongRequest< Type > const * _request )
{
	CPong pong;

	common::CMessage message( pong, _request->getActionKey() );

	m_messages.push_back( message );

		updateLastRequest( _request->getActionKey(), (common::CRequest< Type >const*)_request );
}

}

#endif // NODE_MEDIUM_H
