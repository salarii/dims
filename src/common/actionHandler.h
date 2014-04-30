// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ACTION_HANDLER_H
#define ACTION_HANDLER_H

#include <boost/thread.hpp>

#include <map>

#include "connectionProvider.h"
#include "setResponseVisitor.h"
#include "request.h"
#include "requestHandler.h"

#include <exception>
#include <boost/foreach.hpp>

#include "util.h"
#include "action.h"

namespace common
{

template < class _RequestResponses > class CSetResponseVisitor;
template < class _RequestResponses > class CRequestHandler;
template < class _RequestResponses > struct CRequest;

template < class _RequestResponses >
class CActionHandler
{
public:
	typedef std::multimap<int, CRequestHandler< _RequestResponses > * > AvailableHandlers;
	typedef std::map< CRequest< _RequestResponses >*, CAction< _RequestResponses >* > RequestToAction;
public:
	void loop();
	void shutDown();
	~CActionHandler();
	static CActionHandler* getInstance( );

	void executeAction( CAction< _RequestResponses >* _action );

	void addConnectionProvider( CConnectionProvider< _RequestResponses >* _connectionProvider );
private:
	CActionHandler();

	std::list< CRequestHandler< _RequestResponses > * > provideHandler( int const _request );

	void findAction( CAction< _RequestResponses >* _action ) const;
private:
	static CActionHandler * ms_instance;

	mutable boost::mutex m_mutex;

	std::list< CAction< _RequestResponses >* > m_actions;

	RequestToAction m_reqToAction;

	std::list<CConnectionProvider< _RequestResponses >*> m_connectionProviders;

	//this  will be  multimap one  day, this  should  be  periodically  cleanuped ,  don't  know  how  yet
	AvailableHandlers m_requestHandlers;
	
	static unsigned int const m_sleepTime;
};

template < class _RequestResponses >
CActionHandler< _RequestResponses >::CActionHandler()
{
}

template < class _RequestResponses >
CActionHandler< _RequestResponses >::~CActionHandler()
{
}

template < class _RequestResponses >
CActionHandler< _RequestResponses >*
CActionHandler< _RequestResponses >::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CActionHandler();
	};
	return ms_instance;
}

template < class _RequestResponses >
void
CActionHandler< _RequestResponses >::executeAction( CAction< _RequestResponses >* _action )
{
	boost::lock_guard<boost::mutex> lock(m_mutex);
	m_actions.push_back( _action );
}

template < class _RequestResponses >
void
CActionHandler< _RequestResponses >::addConnectionProvider( CConnectionProvider< _RequestResponses >* _connectionProvider )
{
	m_connectionProviders.push_back( _connectionProvider );
}

template < class _RequestResponses >
std::list< CRequestHandler< _RequestResponses > * >
CActionHandler< _RequestResponses >::provideHandler( int const _requestKind )
{
	std::list< CRequestHandler< _RequestResponses > * > requestHandelers;

	{
		std::pair< typename std::map<int, CRequestHandler< _RequestResponses > * >::iterator, typename std::map<int, CRequestHandler< _RequestResponses > * >::iterator > range;

		 range = m_requestHandlers.equal_range( _requestKind );

		 for ( typename std::map<int, CRequestHandler< _RequestResponses > * >::iterator it = range.first; it != range.second; ++it )
			 requestHandelers.push_back( it->second );

		if ( range.first != m_requestHandlers.end() )
			return requestHandelers;
	}

	typename std::list< CConnectionProvider< _RequestResponses >*>::iterator iterator = m_connectionProviders.begin();

	while( iterator != m_connectionProviders.end() )
	{
		std::list< CMedium< _RequestResponses >*> mediums= (*iterator)->provideConnection( _requestKind );

		if ( !mediums.empty() )
		{
			BOOST_FOREACH( CMedium< _RequestResponses > * medium, mediums )
			{
				CRequestHandler< _RequestResponses > * requestHandler = new CRequestHandler< _RequestResponses >( medium );
				m_requestHandlers.insert( std::make_pair( _requestKind, requestHandler ) );
				requestHandelers.push_back( requestHandler );

			}
			return requestHandelers;
		}

		iterator++;
	}
	return std::list< CRequestHandler< _RequestResponses > * >();
}

template < class _RequestResponses >
void
CActionHandler< _RequestResponses >::shutDown()
{
}

template < class _RequestResponses >
void
CActionHandler< _RequestResponses >::loop()
{
	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock( m_mutex );
			BOOST_FOREACH(CAction< _RequestResponses >* action, m_actions)
			{
				CRequest< _RequestResponses >* request = action->execute();

				if ( request )
					m_reqToAction.insert( std::make_pair( request, action ) );
			}
			m_actions.clear();
		}

		BOOST_FOREACH( typename AvailableHandlers::value_type & reqAction, m_requestHandlers)
		{
			reqAction.second->readLoop();
		}

		std::list< CRequest< _RequestResponses >* > requestsToErase;

		BOOST_FOREACH( typename RequestToAction::value_type & reqAction, m_reqToAction)
		{
			std::list< CRequestHandler< _RequestResponses > * > requestHandlers = provideHandler( reqAction.first->getKind() );

			std::list< _RequestResponses > responses;
			BOOST_FOREACH( CRequestHandler< _RequestResponses > * requestHandler, requestHandlers )
			{
				if ( requestHandler->isProcessed( reqAction.first ) )
				{
					responses.push_back( requestHandler->getResponse( reqAction.first ) );
					requestHandler->deleteRequest( reqAction.first );

					requestsToErase.push_back( reqAction.first );
					delete reqAction.first;
				}
				else
				{
					requestHandler->setRequest( reqAction.first );
				}
			}

				CSetResponseVisitor< _RequestResponses > visitor( responses );
				reqAction.second->accept( visitor );

				m_actions.push_back( reqAction.second );
		}

		BOOST_FOREACH( CRequest< _RequestResponses >* & request, requestsToErase)
		{
			m_reqToAction.erase( request );
		}

		if ( m_reqToAction.empty() )
			boost::this_thread::interruption_point();

		BOOST_FOREACH( typename AvailableHandlers::value_type & reqAction, m_requestHandlers)
		{
			reqAction.second->runRequests();
		}

		MilliSleep( m_sleepTime );
	}
}

}


#endif
