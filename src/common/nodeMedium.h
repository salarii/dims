// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_MEDIUM_H
#define NODE_MEDIUM_H

#include <boost/variant.hpp>

#include "common/medium.h"
#include "common/communicationProtocol.h"
#include "common/commonRequests.h"
#include "common/authenticationProvider.h"
#include "common/selfNode.h"
#include "common/action.h"


namespace common
{
typedef boost::variant< common::CIdentifyMessage > ProtocolMessage;

class CSegmentHeader;
struct CDiskBlock;

template < class _Medium >
class CNodeMedium : public _Medium
{
public:
	typedef typename _Medium::types Type;
	typedef typename Type::Response Response;
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

	void add( CInfoAskRequest< Type > const * _request );

	void add( CStorageInfoRequest< Type > const * _request );

	void add( CSetNextBlockRequest< CSegmentHeader, Type > const * _request );

	void add( CSetNextBlockRequest< CDiskBlock, Type > const * _request );

	void add( CGetBlockRequest< Type > const * _request );

	void add( CValidRegistrationRequest< Type > const * _request );

	void add( CSynchronizationRequest< Type > const * _request );

	void add( CBitcoinHeaderRequest< Type > const * _request );

	void setResponse( uint256 const & _id, Response const & _responses );

	void deleteRequest( CRequest< Type >const* _request );

	common::CSelfNode * getNode() const;

	bool getDirectActionResponseAndClear( CAction< Type >const * _action, std::list< typename Type::Response > & _responses );

	void addActionResponse( uint256 const & _actionId, Response const & _response );
protected:
	void clearResponses();

	void setLastRequest( uint256 const & _id, common::CRequest< Type >const* _request );
protected:
	common::CSelfNode * m_usedNode;

	mutable boost::mutex m_mutex;
	std::multimap< uint256, Response > m_responses;

	static uint256 m_counter;

	std::vector< common::CMessage > m_messages;

	std::set< uint256 > m_indexes;

	std::list< uint256 > m_synchronizeQueue;

	std::map< uint256, common::CRequest< Type >const* > m_idToRequest;

	std::multimap< uint256, Response > m_actionToResponse;
};

template < class _Medium >
bool
CNodeMedium< _Medium >::serviced() const
{
	return !m_responses.empty();
}

template < class _Medium >
void
CNodeMedium< _Medium >::setLastRequest( uint256 const & _id, common::CRequest< Type >const* _request )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	if ( m_idToRequest.find( _id ) != m_idToRequest.end() )
		m_idToRequest.erase( _id );

	m_idToRequest.insert( std::make_pair( _id, _request ) );
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
CNodeMedium< _Medium >::getResponseAndClear( std::multimap< CRequest< Type >const*, Response > & _requestResponse )
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

		typename std::multimap< uint256, Response >::const_iterator iterator = m_responses.lower_bound( *indexIterator );
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

template < class _Medium >
void
CNodeMedium< _Medium >::clearResponses()
{
	BOOST_FOREACH( uint256 const & id, deleteList )
	{
		m_responses.erase( m_responses.lower_bound( id ) );
		//m_idToRequest.erase( id );//doubt if this is ok
		//	m_indexes.erase( id );
	}
	deleteList.clear();
}

template < class _Medium >
void
CNodeMedium< _Medium >::setResponse( uint256 const & _id, Response const & _response )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_responses.insert( std::make_pair( _id, _response ) );
	m_indexes.insert( _id );
	m_synchronizeQueue.push_back( _id );
}

template < class _Medium >
common::CSelfNode *
CNodeMedium< _Medium >::getNode() const
{
	return m_usedNode;
}

template < class _Medium >
void
CNodeMedium< _Medium >::addActionResponse( uint256 const & _actionId, Response const & _response )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_actionToResponse.insert( std::make_pair( _actionId, _response ) );
		m_synchronizeQueue.push_back( _actionId );
}

template < class _Medium >
bool
CNodeMedium< _Medium >::getDirectActionResponseAndClear( CAction< Type >const * _action, std::list< typename Type::Response > & _responses )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	if ( m_synchronizeQueue.empty() )
		return false;

	if ( _action->getActionKey() != m_synchronizeQueue.front() )
		return false;

	m_synchronizeQueue.pop_front();

	typename std::multimap< uint256, Response >::const_iterator iterator
			= m_actionToResponse.lower_bound( _action->getActionKey() );

	while ( iterator != m_actionToResponse.upper_bound( _action->getActionKey() ) )
	{
		_responses.push_back( iterator->second );
		iterator++;
	}

	m_actionToResponse.erase( _action->getActionKey() );
	return true;
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

	common::CMessage message( identifyMessage, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CGetBlockRequest< Type > const * _request )
{
	common::CSynchronizationGet get;

	get.m_number = _request->getBlockNumber();
	get.m_kind = _request->getBlockKind();

	common::CMessage message( get, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest< Type >*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CValidRegistrationRequest< Type > const * _request )
{
	common::CValidRegistration validRegistration;

	validRegistration.m_registrationTime = _request->getContractTime();
	validRegistration.m_period = _request->getPeriod();
	validRegistration.m_key = _request->getKey();

	common::CMessage message( validRegistration, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest< Type >*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CNetworkRoleRequest< Type > const * _request )
{
	common::CNetworkRole networkRole;

	networkRole.m_role = _request->getRole();

	common::CMessage message( networkRole, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CSynchronizationRequest< Type > const * _request )
{
	common::CMessage message( CSynchronizationAsk(), _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CBitcoinHeaderRequest< Type > const * _request )
{
	CBitcoinHeader bitcoinHeader;
	bitcoinHeader.m_bitcoinHeader = _request->getBlockHeader();

	common::CMessage message( bitcoinHeader, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CKnownNetworkInfoRequest< Type > const * _request )
{
	common::CMessage message( _request->getNetworkInfo(), _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

		setLastRequest( _request->getId(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CAckRequest< Type > const * _request )
{
	common::CAck ack;

	common::CMessage message( ack, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

		setLastRequest( _request->getId(), (common::CRequest< Type >const*)_request );
}


template < class _Medium >
void
CNodeMedium< _Medium >::add( CEndRequest< Type > const * _request )
{
	common::CEnd end;

	common::CMessage message( end, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest< Type >const*)_request );//most likely wrong, but handy for time being
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CResultRequest< Type > const * _request )
{
	common::CResult result;

	result.m_result = _request->getResult();

	common::CMessage message( result, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CPingRequest< Type > const * _request )
{
	CPing ping;

	common::CMessage message( ping, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

		setLastRequest( _request->getId(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::deleteRequest( CRequest< Type >const* _request )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_idToRequest.erase( _request->getId() );
	m_responses.erase( _request->getId() );
	m_indexes.erase( _request->getId() );
	m_synchronizeQueue.remove( _request->getId() );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CPongRequest< Type > const * _request )
{
	CPong pong;

	common::CMessage message( pong, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

		setLastRequest( _request->getId(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CInfoAskRequest< Type > const * _request )
{
	CInfoRequestData infoReqData( ( int ) _request->getInfoKind(), _request->getPayload() );

	common::CMessage message( infoReqData, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CStorageInfoRequest< Type > const * _request )
{
	CSynchronizationInfo synchronizationInfo( _request->getTime(), _request->getHeaderSize(), _request->getStorageSize() );

	common::CMessage message( synchronizationInfo, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest< Type >const*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CSetNextBlockRequest< CSegmentHeader, Type > const * _request )
{
	CSynchronizationSegmentHeader synchronizationSegmentHeader( _request->getBlock(), _request->getBlockIndex() );

	CMessage message( synchronizationSegmentHeader, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest< Type >*)_request );
}

template < class _Medium >
void
CNodeMedium< _Medium >::add( CSetNextBlockRequest< CDiskBlock, Type > const * _request )
{
	CSynchronizationBlock synchronizationBlock( _request->getBlock(), _request->getBlockIndex() );

	CMessage message( synchronizationBlock, _request->getActionKey(), _request->getId() );

	m_messages.push_back( message );

	setLastRequest( _request->getId(), (common::CRequest< Type >*)_request );
}

}

#endif // NODE_MEDIUM_H
