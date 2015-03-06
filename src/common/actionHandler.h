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

template < class _RequestResponses > class CSetResponseVisitor;
template < class _RequestResponses > class CRequestHandler;
template < class _RequestResponses > struct CRequest;

template < class _Medium >
struct LessHandlers : public std::binary_function< CRequestHandler< _Medium >* ,CRequestHandler< _Medium >* ,bool>
{
	bool operator() ( CRequestHandler< _Medium >* const & _handlerLhs, CRequestHandler< _Medium >* const & _handlerRhs) const
	{
		return *_handlerLhs < *_handlerRhs;
	}

	bool operator() ( CRequestHandler< _Medium >* const & _handlerLhs, _Medium* const & _medium) const
	{
		return *_handlerLhs < _medium;
	}

	bool operator() ( _Medium* const & _medium, CRequestHandler< _Medium >* const & _handlerLhs ) const
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
	typedef MEDIUM_TYPE(_Types) MediumType;
	typedef RESPONSE_TYPE(_Types) ResponseType;
	typedef FILTER_TYPE(_Types) FilterType;

	typedef std::set< CRequestHandler< MediumType > *, LessHandlers< MediumType > > AvailableHandlers;

	typedef std::map< CRequest< ResponseType >*, CAction< ResponseType >* > RequestToAction;

	typedef std::multimap< CRequest< ResponseType >*, CRequestHandler< MediumType > * > RequestToHandlers;
public:
	void loop();
	void shutDown();
	~CActionHandler();
	static CActionHandler* getInstance( );

	void executeAction( CAction< ResponseType >* _action );

	void addConnectionProvider( CConnectionProvider< FilterType >* _connectionProvider );
private:
	CActionHandler();

	std::list< CRequestHandler< MediumType > * > provideHandler( FilterType const & _filter );

	void findAction( CAction< ResponseType >* _action ) const;

private:
	static CActionHandler * ms_instance;

	mutable boost::mutex m_mutex;

	std::set< CAction< ResponseType >* > m_actions;

	RequestToAction m_reqToAction;

	std::map< CAction< ResponseType >*, std::set< CRequest< ResponseType >* > > m_actionToExecutedRequests;

	std::list<CConnectionProvider< FilterType >*> m_connectionProviders;

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
		ms_instance = new CActionHandler();
	};
	return ms_instance;
}

template < class _Types >
void
CActionHandler< _Types >::executeAction( CAction< ResponseType >* _action )
{
	boost::lock_guard<boost::mutex> lock(m_mutex);

	if ( !_action->isInProgress() )
	{
			_action->setInProgress();
			m_actions.insert( _action );
	}
}

template < class _Types >
void
CActionHandler< _Types >::addConnectionProvider( CConnectionProvider< FilterType >* _connectionProvider )
{
	m_connectionProviders.push_back( _connectionProvider );
}

/* I have  done some drastic simplification here, in case  of problems look how it  was  done before till August 7-8 */
template < class _Types >
std::list< CRequestHandler< typename CActionHandler<_Types>::MediumType > * >
CActionHandler< _Types >::provideHandler( FilterType const & _filter )
{
	std::list< CRequestHandler< MediumType > * > requestHandelers;

	typename std::list< CConnectionProvider< FilterType >*>::iterator iterator = m_connectionProviders.begin();

	while( iterator != m_connectionProviders.end() )// for now only one provider is allowed to service request so firs is best
	{
		std::list< MediumType*> mediums= (*iterator)->provideConnection( _filter );

		if ( !mediums.empty() )
		{
			BOOST_FOREACH( MediumType * medium, mediums )
			{
				typename AvailableHandlers::iterator iterator = std::lower_bound( m_requestHandlers.begin(), m_requestHandlers.end(), medium, LessHandlers< MediumType >() );
				if ( iterator != std::upper_bound( m_requestHandlers.begin(), m_requestHandlers.end(), medium, LessHandlers< MediumType >() ) )
				{
					requestHandelers.push_back( *iterator );
				}
				else
				{
					CRequestHandler< MediumType > * requestHandler = new CRequestHandler< MediumType >( medium );
					m_requestHandlers.insert( requestHandler );
					requestHandelers.push_back( requestHandler );
				}
			}
			return requestHandelers;
		}

		iterator++;
	}
	return std::list< CRequestHandler< MediumType > * >();
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
	std::set< CRequestHandler< MediumType > * > requestHandlersToExecute;
	std::set< CRequestHandler< MediumType > * > requestHandlersToRead;
	while(1)
	{

		{
			boost::lock_guard<boost::mutex> lock( m_mutex );
			BOOST_FOREACH(CAction< ResponseType >* action, m_actions)
			{

				std::vector< CRequest< ResponseType >* > requests = action->getRequests();

				if ( !requests.empty() )
				{
					BOOST_FOREACH( CRequest< ResponseType >* request, requests )
					{
						m_reqToAction.insert( std::make_pair( request, action ) );

						typename RequestToHandlers::iterator lower = m_currentlyUsedHandlers.lower_bound (request);
						typename RequestToHandlers::iterator upper = m_currentlyUsedHandlers.upper_bound (request);

						if ( lower == upper )
						{
							std::list< CRequestHandler< MediumType > * > requestHandlers = provideHandler( *request->getMediumFilter() );

							BOOST_FOREACH( CRequestHandler< MediumType > * requestHandler, requestHandlers )
							{
								requestHandlersToExecute.insert( requestHandler );
								requestHandler->setRequest( request );

								m_currentlyUsedHandlers.insert( std::make_pair( request, requestHandler ) );
							}

						}

					}
				}
				else
				{
					if ( action->autoDelete() )
					{
						// clean m_currentlyUsedHandlers here
						delete action;
					}
					else
						action->setExecuted();
				}

			}
			m_actions.clear();
		}

		BOOST_FOREACH( CRequestHandler< MediumType > * reqHandler, requestHandlersToRead )
		{
			reqHandler->processMediumResponses();
		}

		std::list< CRequest< ResponseType >* > requestsToErase;

		BOOST_FOREACH( typename RequestToAction::value_type & reqAction, m_reqToAction)
		{
			typename RequestToHandlers::iterator lower = m_currentlyUsedHandlers.lower_bound (reqAction.first);
			typename RequestToHandlers::iterator upper = m_currentlyUsedHandlers.upper_bound (reqAction.first);
			for ( typename RequestToHandlers::iterator it = lower; it!=upper; ++it)
			{
				if ( it->second->isProcessed( reqAction.first ) )
				{
					std::vector< ResponseType > responses = it->second->getResponses( reqAction.first );

					BOOST_FOREACH( ResponseType const & response, responses )
					{
						CSetResponseVisitor< ResponseType > visitor( response );
						reqAction.second->accept( visitor );
					}
					m_actions.insert( reqAction.second );
					it->second->deleteRequest( reqAction.first );
					requestsToErase.push_back( reqAction.first );
				}
				else
				{
					// problem ??? assert this??
				}
			}
		}

		BOOST_FOREACH( CRequest< ResponseType >* & request, requestsToErase)
		{
			m_reqToAction.erase( request );
		}

		if ( m_reqToAction.empty() )
			boost::this_thread::interruption_point();

		BOOST_FOREACH( CRequestHandler< MediumType > * reqHandler, requestHandlersToExecute )
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
