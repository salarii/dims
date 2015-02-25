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

#include <exception>
#include <boost/foreach.hpp>

#include "util.h"
#include "action.h"
#include <algorithm>
/*
initially I wanted to make every actionHandler to operate on its own response list
I feared that if  I put everything in the same mpl::list it will be bloated to impossibility,
additionally I thought that it is  obvious that every action  handler  should have distinctive features.

now I doubt
this approach lead me to  situation where I have to drag template parameters like ( NodeResponses, TrackerResponses etc. )everywhere
I didn't expected that my initial choice will have such devastating  result to the code  apperance
I am on the brink of removing separate response lists completely, this will result in removing template  parameter from most places

*/
/*
I will try to  redesign  this  mechanics
what I want  to  achieve is among  others
bond responses  to  request  not  action  it  means  that no metter  how many responses  are  there for  single  request
they will be  processed  anyway.
No more  effort to retrive them  no  more  pending or  continue request
so ask medium in some  smart  way and if  they  have  something process it
match every response  against currently existing request if it exists process it if  not  drop
this  should remove  a lot of  complexity
allow  action  to produce many  requests  at the same  time
for exaple  iming  request  and  some  operation  request
*/
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

	bool operator() ( CMedium< _RequestResponses >* const & _medium, CRequestHandler< _RequestResponses >* const & _handlerLhs ) const
	{
		if ( *_handlerLhs < _medium )
			return false;

		if ( *_handlerLhs == _medium )
			return false;

		return true;
	}
};


template < class _RequestResponses >
class CActionHandler
{
public:
	typedef std::set< CRequestHandler< _RequestResponses > *, LessHandlers< _RequestResponses > > AvailableHandlers;

	typedef std::map< CRequest< _RequestResponses >*, CAction< _RequestResponses >* > RequestToAction;

	typedef std::multimap< CRequest< _RequestResponses >*, CRequestHandler< _RequestResponses > * > RequestToHandlers;
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

	std::set< CAction< _RequestResponses >* > m_actions;

	RequestToAction m_reqToAction;

	std::map< CAction< _RequestResponses >*, std::set< CRequest< _RequestResponses >* > > m_actionToExecutedRequests;

	std::list<CConnectionProvider< _RequestResponses >*> m_connectionProviders;

	AvailableHandlers m_requestHandlers;

	static unsigned int const m_sleepTime;

	RequestToHandlers m_currentlyUsedHandlers;
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

	if ( !_action->isInProgress() )
	{
			_action->setInProgress();
			m_actions.insert( _action );
	}
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
				if ( iterator != std::upper_bound( m_requestHandlers.begin(), m_requestHandlers.end(), medium, LessHandlers< _RequestResponses >() ) )
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

				std::vector< CRequest< _RequestResponses >* > requests = action->getRequests();

				if ( !requests.empty() )
				{
					BOOST_FOREACH( CRequest< _RequestResponses >* request, requests )
					{
						m_reqToAction.insert( std::make_pair( request, action ) );
					}
				}
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
			reqHandler->processMediumResponses();
		}

		std::list< CRequest< _RequestResponses >* > requestsToErase;

		BOOST_FOREACH( typename RequestToAction::value_type & reqAction, m_reqToAction)
		{

			typename RequestToHandlers::iterator lower = m_currentlyUsedHandlers.lower_bound (reqAction.first);
			typename RequestToHandlers::iterator upper = m_currentlyUsedHandlers.upper_bound (reqAction.first);

			if ( lower == upper )
			{
				std::list< CRequestHandler< _RequestResponses > * > requestHandlers = provideHandler( *reqAction.first->getMediumFilter() );

				BOOST_FOREACH( CRequestHandler< _RequestResponses > * requestHandler, requestHandlers )
				{
					requestHandlersToExecute.insert( requestHandler );
					requestHandler->setRequest( reqAction.first );

					m_currentlyUsedHandlers.insert( std::make_pair( reqAction.first, requestHandler ) );
				}

			}
			else
			{
				for ( typename RequestToHandlers::iterator it = lower; it!=upper; ++it)
				{
					if ( it->second->isProcessed( reqAction.first ) )
					{
						std::vector< _RequestResponses > responses = it->second->getResponses( reqAction.first );

						BOOST_FOREACH( _RequestResponses const & response, responses )
						{
							CSetResponseVisitor< _RequestResponses > visitor( response );
							reqAction.second->accept( visitor );
						}
						m_actions.insert( reqAction.second );
						it->second->deleteRequest( reqAction.first );
						requestsToErase.push_back( reqAction.first );

						m_currentlyUsedHandlers.erase( it );
					}
					else
					{
						// problem ??? assert this??
					}
				}
			}
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
