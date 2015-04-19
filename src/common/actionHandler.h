// Copyright (c) 2014-2015 Dims dev-team
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
#include "common/types.h"

#include <exception>
#include <boost/foreach.hpp>

#include "util.h"
#include "action.h"
#include <algorithm>

namespace common
{

template < class _Type > class CSetResponseVisitor;
template < class _Type > class CRequestHandler;
template < class _Type > class CRequest;

template < class _Type >
struct LessHandlers : public std::binary_function< CRequestHandler< _Type >* ,CRequestHandler< _Type >* ,bool>
{
	bool operator() ( CRequestHandler< _Type >* const & _handlerLhs, CRequestHandler< _Type >* const & _handlerRhs) const
	{
		return *_handlerLhs < *_handlerRhs;
	}

	bool operator() ( CRequestHandler< _Type >* const & _handlerLhs, typename _Type::Medium* const & _medium) const
	{
		return *_handlerLhs < _medium;
	}

	bool operator() ( typename _Type::Medium* const & _medium, CRequestHandler< _Type >* const & _handlerLhs ) const
	{
		if ( *_handlerLhs < _medium )
			return false;

		if ( *_handlerLhs == _medium )
			return false;

		return true;
	}
};


template < class _Types >
class CActionHandler
{
public:
	typedef typename _Types::Medium MediumType;
	typedef typename _Types::Response ResponseType;
	typedef typename _Types::Filter FilterType;

	typedef std::set< CRequestHandler< _Types > *, LessHandlers< _Types > > AvailableHandlers;

	typedef std::map< CRequest< _Types >*, CAction< _Types >* > RequestToAction;

	typedef std::multimap< CRequest< _Types >*, CRequestHandler< _Types > * > RequestToHandlers;
public:
	void loop();
	void shutDown();
	~CActionHandler();
	static CActionHandler* getInstance( );

	void executeAction( CAction< _Types >* _action );

	void addConnectionProvider( CConnectionProvider< _Types >* _connectionProvider );
private:
	CActionHandler();

	std::list< CRequestHandler< _Types > * > provideHandler( FilterType const & _filter );

	void findAction( CAction< _Types >* _action ) const;

private:
	static CActionHandler * ms_instance;

	mutable boost::mutex m_mutex;

	std::set< CAction< _Types >* > m_actions;

		std::set< CAction< _Types >* > m_allActions;

	RequestToAction m_reqToAction;

	std::map< CAction< _Types >*, std::set< CRequest< _Types >* > > m_actionToExecutedRequests;

	std::list<CConnectionProvider< _Types >*> m_connectionProviders;

	AvailableHandlers m_requestHandlers;

	static unsigned int const m_sleepTime;

	RequestToHandlers m_currentlyUsedHandlers;// clean this  up when an action dies
};

template < class _Types >
CActionHandler< _Types >::CActionHandler()
{
}

template < class _Types >
CActionHandler< _Types >::~CActionHandler()
{
}

template < class _Types >
CActionHandler< _Types >*
CActionHandler< _Types >::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CActionHandler< _Types >();
	};
	return ms_instance;
}

template < class _Types >
void
CActionHandler< _Types >::executeAction( CAction< _Types >* _action )
{
	boost::lock_guard<boost::mutex> lock(m_mutex);

	if ( !_action->isInProgress() )
	{
			_action->setInProgress();
			m_actions.insert( _action );
			m_allActions.insert( _action );
	}
}

template < class _Types >
void
CActionHandler< _Types >::addConnectionProvider( CConnectionProvider< _Types >* _connectionProvider )
{
	m_connectionProviders.push_back( _connectionProvider );
}

/* I have  done some drastic simplification here, in case  of problems look how it  was  done before till August 7-8 */
template < class _Types >
std::list< CRequestHandler< _Types > * >
CActionHandler< _Types >::provideHandler( FilterType const & _filter )
{
	std::list< CRequestHandler< _Types > * > requestHandelers;

	typename std::list< CConnectionProvider< _Types >*>::iterator iterator = m_connectionProviders.begin();

	while( iterator != m_connectionProviders.end() )// for now only one provider is allowed to service request so firs is best
	{
		std::list< MediumType*> mediums= (*iterator)->provideConnection( _filter );

		if ( !mediums.empty() )
		{
			BOOST_FOREACH( MediumType * medium, mediums )
			{
				typename AvailableHandlers::iterator iterator = std::lower_bound( m_requestHandlers.begin(), m_requestHandlers.end(), medium, LessHandlers< _Types >() );
				if ( iterator != std::upper_bound( m_requestHandlers.begin(), m_requestHandlers.end(), medium, LessHandlers< _Types >() ) )
				{
					requestHandelers.push_back( *iterator );
				}
				else
				{
					CRequestHandler< _Types > * requestHandler = new CRequestHandler< _Types >( medium );
					m_requestHandlers.insert( requestHandler );
					requestHandelers.push_back( requestHandler );
				}
			}
			return requestHandelers;
		}

		iterator++;
	}
	return std::list< CRequestHandler< _Types > * >();
}

template < class _Types >
void
CActionHandler< _Types >::shutDown()
{
}

template < class _Types >
void
CActionHandler< _Types >::loop()
{
	std::set< CRequestHandler< _Types > * > requestHandlersToExecute;
	std::set< CRequestHandler< _Types > * > requestHandlersToRead;
	while(1)
	{

		{
			boost::lock_guard<boost::mutex> lock( m_mutex );
			std::list< CAction< _Types >* > toErase;
			BOOST_FOREACH(CAction< _Types >* action, m_actions)
			{
				std::vector< CRequest< _Types >* > requests = action->getRequests();

				if ( action->needToExit() )
				{
					toErase.push_back( action );

					if ( action->autoDelete() )
					{
						std::vector< CRequest< _Types >* > dropped = action->getDroppedRequests();
						BOOST_FOREACH( CRequest< _Types >* request,dropped )
						{
							typename RequestToHandlers::iterator lower = m_currentlyUsedHandlers.lower_bound ( request );
							typename RequestToHandlers::iterator upper = m_currentlyUsedHandlers.upper_bound ( request );
							for ( typename RequestToHandlers::iterator it = lower; it!=upper; ++it)
							{
								it->second->deleteRequest( request );
							}
							m_currentlyUsedHandlers.erase( request );
						}
						m_allActions.erase( action );

						delete action;
					}
					else
						action->setExecuted();
				}
				else if ( !requests.empty() )
				{
					toErase.push_back( action );
					std::vector< CRequest< _Types >* > combined;

					combined = action->getDroppedRequests();

					combined.insert( combined.end(), requests.begin(), requests.end() );

					BOOST_FOREACH( CRequest< _Types >* request, requests )
					{
						m_reqToAction.insert( std::make_pair( request, action ) );
					}

					BOOST_FOREACH( CRequest< _Types >* request, combined )
					{
						m_reqToAction.insert( std::make_pair( request, action ) );

						typename RequestToHandlers::iterator lower = m_currentlyUsedHandlers.lower_bound (request);
						typename RequestToHandlers::iterator upper = m_currentlyUsedHandlers.upper_bound (request);

						if ( lower == upper )
						{
							std::list< CRequestHandler< _Types > * > requestHandlers = provideHandler( *request->getMediumFilter() );

							BOOST_FOREACH( CRequestHandler< _Types > * requestHandler, requestHandlers )
							{
								requestHandlersToExecute.insert( requestHandler );
								requestHandler->setRequest( request );

								m_currentlyUsedHandlers.insert( std::make_pair( request, requestHandler ) );
							}

						}

					}
				}
			}
			BOOST_FOREACH( CAction< _Types >* action, toErase )
			{
				m_actions.erase( action );
			}
		}

		BOOST_FOREACH( CRequestHandler< _Types > * reqHandler, requestHandlersToRead )
		{
			BOOST_FOREACH( CAction< _Types >* action,m_allActions )
			{
				std::list< ResponseType > responses = reqHandler->getDirectActionResponse( action );
				BOOST_FOREACH( ResponseType const & response, responses )
				{
					CSetResponseVisitor< _Types > visitor( response );
					action->accept( visitor );
					m_actions.insert( action );
				}
			}
		}

		BOOST_FOREACH( CRequestHandler< _Types > * reqHandler, requestHandlersToRead )
		{
			reqHandler->processMediumResponses();
		}

		std::set< CAction< _Types >* > actionsToErase;

		std::multimap< CAction< _Types >*, CRequest< _Types > * > eraseCandidates;

		BOOST_FOREACH( typename RequestToAction::value_type & reqAction, m_reqToAction)
		{
			eraseCandidates.insert( std::make_pair( reqAction.second, reqAction.first ) );
			typename RequestToHandlers::iterator lower = m_currentlyUsedHandlers.lower_bound (reqAction.first);
			typename RequestToHandlers::iterator upper = m_currentlyUsedHandlers.upper_bound (reqAction.first);
			for ( typename RequestToHandlers::iterator it = lower; it!=upper; ++it)
			{
				if ( it->second->isProcessed( reqAction.first ) )
				{
					std::vector< ResponseType > responses = it->second->getResponses( reqAction.first );

					BOOST_FOREACH( ResponseType const & response, responses )
					{
						CSetResponseVisitor< _Types > visitor( response );
						reqAction.second->accept( visitor );
					}
					m_actions.insert( reqAction.second );
					actionsToErase.insert( reqAction.second );
					it->second->deleteRequest( reqAction.first );
				}
				else
				{
					// problem ??? assert this??
				}
			}
		}

		BOOST_FOREACH( CAction< _Types >* const & action, actionsToErase)
		{
			typename std::multimap< CAction< _Types >*, CRequest< _Types > * >::const_iterator lower
					= eraseCandidates.lower_bound ( action );
			typename std::multimap< CAction< _Types >*, CRequest< _Types > * >::const_iterator upper
					= eraseCandidates.upper_bound ( action );

			for ( typename std::multimap< CAction< _Types >*, CRequest< _Types > * >::const_iterator it = lower; it!=upper; ++it)
			{
				m_reqToAction.erase( it->second );
			}

		}

		if ( m_reqToAction.empty() )
			boost::this_thread::interruption_point();

		BOOST_FOREACH( CRequestHandler< _Types > * reqHandler, requestHandlersToExecute )
		{
			reqHandler->runRequests();
			requestHandlersToRead.insert( reqHandler );
		}

		requestHandlersToExecute.clear();
		MilliSleep( m_sleepTime );
	}
}

}


#endif
