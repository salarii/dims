// Copyright (c) 2014 Dims dev-team
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
#include <algorithm>

namespace common
{

template < class _RequestResponses > class CSetResponseVisitor;
template < class _RequestResponses > class CRequestHandler;
template < class _RequestResponses > struct CRequest;

template < class _RequestResponses >
struct LessHandlers : public std::binary_function< CRequestHandler< _RequestResponses >* ,CRequestHandler< _RequestResponses >* ,bool>
{
	bool operator() ( CRequestHandler< _RequestResponses >* const & _handlerLhs, CRequestHandler< _RequestResponses >* const & _handlerRhs) const
	{
		return *_handlerLhs < *_handlerRhs;
	}

	bool operator() ( CRequestHandler< _RequestResponses >* const & _handlerLhs, CMedium< _RequestResponses >* const & _medium) const
	{
		return *_handlerLhs < _medium;
	}
};


template < class _RequestResponses >
class CActionHandler
{
public:
	typedef std::set< CRequestHandler< _RequestResponses > *, LessHandlers< _RequestResponses > > AvailableHandlers;

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

	std::list< CRequestHandler< _RequestResponses > * > provideHandler( CMediumFilter<_RequestResponses> const & _mediumFilter );

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

/* I have  done some drastic simplification here, in case  of problems look how it  was  done before till August 7-8 */
template < class _RequestResponses >
std::list< CRequestHandler< _RequestResponses > * >
CActionHandler< _RequestResponses >::provideHandler( CMediumFilter<_RequestResponses> const & _mediumFilter )
{
	std::list< CRequestHandler< _RequestResponses > * > requestHandelers;

	typename std::list< CConnectionProvider< _RequestResponses >*>::iterator iterator = m_connectionProviders.begin();

	while( iterator != m_connectionProviders.end() )// for now only one provider is allowed to service request so firs is best
	{
		std::list< CMedium< _RequestResponses >*> mediums= (*iterator)->provideConnection( _mediumFilter );

		if ( !mediums.empty() )
		{
			BOOST_FOREACH( CMedium< _RequestResponses > * medium, mediums )
			{
				// here we are counting on medium provider to select properly
				typename AvailableHandlers::iterator iterator = std::lower_bound( m_requestHandlers.begin(), m_requestHandlers.end(), medium, LessHandlers< _RequestResponses >() );
				if ( iterator != m_requestHandlers.end() )
				{
					requestHandelers.push_back( *iterator );
				}
				else
				{
					CRequestHandler< _RequestResponses > * requestHandler = new CRequestHandler< _RequestResponses >( medium );
					m_requestHandlers.insert( requestHandler );
					requestHandelers.push_back( requestHandler );
				}
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
	std::set< CRequestHandler< _RequestResponses > * > requestHandlersToExecute;
	while(1)
	{

		{
			boost::lock_guard<boost::mutex> lock( m_mutex );
			BOOST_FOREACH(CAction< _RequestResponses >* action, m_actions)
			{

				CRequest< _RequestResponses >* request = action->execute();

				if ( request )
					m_reqToAction.insert( std::make_pair( request, action ) );
				else
				{
					if ( action->autoDelete() )
						delete action;
					else
						action->setExecuted();
				}
				// for safety  reason one should  consider reseting  current request to  0
			}
			m_actions.clear();
		}

		BOOST_FOREACH( CRequestHandler< _RequestResponses > * reqHandler, requestHandlersToExecute )
		{
			reqHandler->readLoop();
		}
		requestHandlersToExecute.clear();

		std::list< CRequest< _RequestResponses >* > requestsToErase;

		BOOST_FOREACH( typename RequestToAction::value_type & reqAction, m_reqToAction)
		{
			std::list< CRequestHandler< _RequestResponses > * > requestHandlers = provideHandler( *reqAction.first->getMediumFilter() );

			std::list< _RequestResponses > responses;
			bool deleteRequest = false;
			BOOST_FOREACH( CRequestHandler< _RequestResponses > * requestHandler, requestHandlers )
			{
				if ( requestHandler->isProcessed( reqAction.first ) )
				{
					_RequestResponses response = requestHandler->getResponse( reqAction.first );

					CSetResponseVisitor< _RequestResponses > visitor( response );
					reqAction.second->accept( visitor );

					m_actions.push_back( reqAction.second );

					requestHandler->deleteRequest( reqAction.first );

					requestsToErase.push_back( reqAction.first );

					deleteRequest = true;
				}
				else
				{
					requestHandlersToExecute.insert( requestHandler );
					requestHandler->setRequest( reqAction.first );
				}
			}
			if ( deleteRequest )
				delete reqAction.first;
		}

		BOOST_FOREACH( CRequest< _RequestResponses >* & request, requestsToErase)
		{
			m_reqToAction.erase( request );
		}

		if ( m_reqToAction.empty() )
			boost::this_thread::interruption_point();

		BOOST_FOREACH( CRequestHandler< _RequestResponses > * reqHandler, requestHandlersToExecute )
		{
			reqHandler->runRequests();
		}

		MilliSleep( m_sleepTime );
	}
}

}


#endif
