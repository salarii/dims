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


struct CMatcher
{
	CMatcher( int _key )
	{
		m_match.push_back( _key );
	}

	bool add( int _key )
	{
		BOOST_FOREACH( int key, m_match )
		{
			if ( key == _key )
				return false;
		}

		m_match.push_back( _key );
		std::sort( m_match.begin(), m_match.end() );
		return true;
	}
	CMatcher & operator=( CMatcher const & _matcher )
	{
		m_match = _matcher.m_match;
		return *this;
	}

	bool operator!=( CMatcher const & _matcher ) const
	{
		return m_match != _matcher.m_match;
	}
	bool operator<( CMatcher const & _matcher ) const
	{
		bool less = m_match < _matcher.m_match;
		return m_match < _matcher.m_match;
	}
	std::vector< int > m_match;
};

template < class _RequestResponses >
class CActionHandler
{
public:
	typedef std::multimap< CMatcher, CRequestHandler< _RequestResponses > * > AvailableHandlers;
	std::set< CMatcher > m_complexMatchers;
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

	std::vector< CMatcher > findComplexMatchers( int _key );
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

struct CFindMatcher : unary_function < CMatcher, bool >
{
	CFindMatcher( int _key ):m_key(_key){};
	bool operator() ( CMatcher const & _matcher ) const
	{
		return std::find (_matcher.m_match.begin(), _matcher.m_match.end(), m_key) == _matcher.m_match.end();
	}

	int const m_key;
};


//hate logic like this, but it seems I need it
template < class _RequestResponses >
std::vector< CMatcher >
CActionHandler< _RequestResponses >::findComplexMatchers( int _key )
{
	std::vector< CMatcher > matchers;
	std::copy(m_complexMatchers.begin(), m_complexMatchers.end(), std::back_inserter(matchers));
	std::remove_if( matchers.begin(),matchers.end(), CFindMatcher( _key ) );
	return matchers;
}

template < class _RequestResponses >
std::list< CRequestHandler< _RequestResponses > * >
CActionHandler< _RequestResponses >::provideHandler( int const _requestKind )
{
	std::list< CRequestHandler< _RequestResponses > * > requestHandelers;

	{

		std::vector< CMatcher > complexMatchers = findComplexMatchers( _requestKind );
		complexMatchers.push_back( _requestKind );
		BOOST_FOREACH( CMatcher const & matcher, complexMatchers )
		{
			std::pair< typename AvailableHandlers::iterator, typename AvailableHandlers::iterator > range;

			 range = m_requestHandlers.equal_range( matcher );

			 for ( typename AvailableHandlers::iterator it = range.first; it != range.second; ++it )
				 requestHandelers.push_back( it->second );
		}


		if ( !requestHandelers.empty() )
			return requestHandelers;
	}

	typename std::list< CConnectionProvider< _RequestResponses >*>::iterator iterator = m_connectionProviders.begin();

	while( iterator != m_connectionProviders.end() )
	{
		std::list< CMedium< _RequestResponses >*> mediums= (*iterator)->provideConnection( _requestKind );

		if ( !mediums.empty() )
		{
			typedef std::pair< CMatcher, CRequestHandler< _RequestResponses > * > MatchedHandler;
			BOOST_FOREACH( CMedium< _RequestResponses > * medium, mediums )
			{
				typename AvailableHandlers::iterator iterator = m_requestHandlers.begin();


				std::vector< MatchedHandler >matchedHandlers;
				while ( iterator != m_requestHandlers.end() )
				{
					typename AvailableHandlers::iterator previous = iterator;

					iterator++;
					if ( *previous->second == medium )
					{
						std::pair< CMatcher, CRequestHandler< _RequestResponses > * > value = *previous;
						matchedHandlers.push_back( *previous );
						m_requestHandlers.erase( previous );
					}
				}

				if ( !matchedHandlers.empty() )
				{

					BOOST_FOREACH( MatchedHandler & matched, matchedHandlers )
					{
						CMatcher newMatch = matched.first;
						if ( newMatch.add( _requestKind ) )
							m_complexMatchers.insert( newMatch );

						m_requestHandlers.insert( std::make_pair( newMatch, matched.second ) );
						requestHandelers.push_back( matched.second );
					}
				}
				else
				{
					CRequestHandler< _RequestResponses > * requestHandler = new CRequestHandler< _RequestResponses >( medium );
					m_requestHandlers.insert( std::make_pair( _requestKind, requestHandler ) );
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
					_RequestResponses response = requestHandler->getResponse( reqAction.first );

					CSetResponseVisitor< _RequestResponses > visitor( response );
					reqAction.second->accept( visitor );

					m_actions.push_back( reqAction.second );

					requestHandler->deleteRequest( reqAction.first );

					requestsToErase.push_back( reqAction.first );
					delete reqAction.first;
				}
				else
				{
					requestHandler->setRequest( reqAction.first );
				}
			}
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
