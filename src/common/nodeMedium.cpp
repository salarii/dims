// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/medium.h"
#include "common/communicationProtocol.h"
#include "common/authenticationProvider.h"
#include "common/action.h"

#include "common/nodeMedium.h"

namespace common
{

bool
CNodeMedium::serviced() const
{
	return !m_responses.empty();
}

void
CNodeMedium::setLastRequest( uint256 const & _id, common::CRequest const* _request )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	if ( m_idToRequest.find( _id ) != m_idToRequest.end() )
		m_idToRequest.erase( _id );

	m_idToRequest.insert( std::make_pair( _id, _request ) );
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

extern std::vector< uint256 > deleteList;

bool
CNodeMedium::getResponseAndClear( std::multimap< CRequest const*, DimsResponse > & _requestResponse )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	if ( m_synchronizeQueue.empty() )
		return false;

	std::set< uint256 >::const_iterator indexIterator = m_indexes.begin();
	while( indexIterator != m_indexes.end() )
	{
		if ( *indexIterator != m_synchronizeQueue.front() )
		{
			indexIterator++;
			continue;
		}

		m_synchronizeQueue.pop_front();

		typename std::multimap< uint256, DimsResponse >::const_iterator iterator = m_responses.lower_bound( *indexIterator );
		while ( iterator != m_responses.upper_bound( *indexIterator ) )
		{
			_requestResponse.insert( std::make_pair( m_idToRequest.find( *indexIterator )->second, iterator->second ) );
			deleteList.push_back( *indexIterator );
			++iterator;
		}

		indexIterator++;
	}
	clearResponses();
	return true;
}

void
CNodeMedium::clearResponses()
{
	BOOST_FOREACH( uint256 const & id, deleteList )
	{
		m_responses.erase( m_responses.lower_bound( id ) );
		//m_idToRequest.erase( id );//doubt if this is ok
		//m_indexes.erase( id );
	}
	deleteList.clear();
}

void
CNodeMedium::setResponse( uint256 const & _id, DimsResponse const & _response )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_responses.insert( std::make_pair( _id, _response ) );
	m_indexes.insert( _id );
	m_synchronizeQueue.push_back( _id );
}

common::CSelfNode *
CNodeMedium::getNode() const
{
	return m_usedNode;
}

void
CNodeMedium::addActionResponse( uint256 const & _actionId, DimsResponse const & _response )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_actionToResponse.insert( std::make_pair( _actionId, _response ) );
	m_synchronizeQueue.push_back( _actionId );
}

void
CNodeMedium::deleteAction( CAction const * _action )
{
	m_actionToResponse.erase( _action->getActionKey() );
	m_synchronizeQueue.remove( _action->getActionKey() );
}

bool
CNodeMedium::getDirectActionResponseAndClear( CAction const * _action, std::list< common::DimsResponse > & _responses )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	if ( m_synchronizeQueue.empty() )
		return false;

	if ( _action->getActionKey() != m_synchronizeQueue.front() )
	{
		return false;
	}
	m_synchronizeQueue.pop_front();

	typename std::multimap< uint256, DimsResponse >::const_iterator iterator
			= m_actionToResponse.lower_bound( _action->getActionKey() );

	while ( iterator != m_actionToResponse.upper_bound( _action->getActionKey() ) )
	{
		_responses.push_back( iterator->second );
		iterator++;
	}

	m_actionToResponse.erase( _action->getActionKey() );
	return true;
}

void
CNodeMedium::add( common::CRequest const * _request )
{
}

void
CNodeMedium::add( CSendIdentifyDataRequest const * _request )
{
	common::CIdentifyMessage identifyMessage;

	identifyMessage.m_payload = _request->getPayload();

	identifyMessage.m_signed = _request->getSigned();

	identifyMessage.m_key = _request->getKey();

	common::CMessage message( identifyMessage, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest const*)_request );
}

void
CNodeMedium::add( CAckRequest const * _request )
{
	common::CAck ack;

	common::CMessage message( ack, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

		setLastRequest( _request->getId(), (common::CRequest const*)_request );
}

void
CNodeMedium::deleteRequest( CRequest const* _request )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_idToRequest.erase( _request->getId() );
	m_responses.erase( _request->getId() );
	m_indexes.erase( _request->getId() );
	m_synchronizeQueue.remove( _request->getId() );
}

void
CNodeMedium::add( CInfoAskRequest const * _request )
{
	CInfoRequestData infoReqData( ( int ) _request->getInfoKind(), _request->getPayload() );

	common::CMessage message( infoReqData, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest const*)_request );
}

void
CNodeMedium::add( CSendMessageRequest const * _request )
{
	common::CMessage message(
				_request->getMessageKind()
				, _request->getPayLoad()
				, _request->getActionKey()
				, _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest const*)_request );
}

}
