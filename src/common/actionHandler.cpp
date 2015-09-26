// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <map>
#include <exception>
#include <boost/foreach.hpp>

#include "util.h"
#include "action.h"
#include <algorithm>

#include "common/connectionProvider.h"
#include "common/setResponseVisitor.h"
#include "common/request.h"
#include "common/requestHandler.h"
#include "common/actionHandler.h"

namespace common
{

bool
LessHandlers::operator() ( CRequestHandler * const & _handlerLhs, CRequestHandler * const & _handlerRhs) const
{
	return *_handlerLhs < *_handlerRhs;
}

bool
LessHandlers::operator() ( CRequestHandler * const & _handlerLhs, CMedium* const & _medium) const
{
	return *_handlerLhs < _medium;
}

bool
LessHandlers::operator() ( CMedium* const & _medium, CRequestHandler * const & _handlerLhs ) const
{
	if ( *_handlerLhs < _medium )
		return false;

	if ( *_handlerLhs == _medium )
		return false;

	return true;
}

unsigned int const common::CActionHandler::m_sleepTime = 100;
common::CActionHandler * common::CActionHandler::ms_instance = NULL;

CActionHandler::CActionHandler()
{
}

CActionHandler::~CActionHandler()
{
}

CActionHandler *
CActionHandler::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CActionHandler ();
	};
	return ms_instance;
}

void
CActionHandler::executeAction( CAction * _action )
{
	boost::lock_guard<boost::mutex> lock(m_mutex);

	if ( !_action->isInProgress() )
	{
			_action->setInProgress();
			m_actions.insert( _action );
	}
}

void
CActionHandler::addConnectionProvider( CConnectionProvider * _connectionProvider )
{
	m_connectionProviders.push_back( _connectionProvider );
}

std::list< CRequestHandler * >
CActionHandler::provideHandler( CMediumFilter const & _filter )
{
	std::list< CRequestHandler * > requestHandelers;

	typename std::list< CConnectionProvider *>::iterator iterator = m_connectionProviders.begin();

	while( iterator != m_connectionProviders.end() )// for now only one provider is allowed to service request so firs is best
	{
		std::list< CMedium*> mediums= (*iterator)->provideConnection( _filter );

		if ( !mediums.empty() )
		{
			BOOST_FOREACH( CMedium * medium, mediums )
			{
				typename AvailableHandlers::iterator iterator = std::lower_bound( m_requestHandlers.begin(), m_requestHandlers.end(), medium, LessHandlers () );
				if ( iterator != std::upper_bound( m_requestHandlers.begin(), m_requestHandlers.end(), medium, LessHandlers () ) )
				{
					requestHandelers.push_back( *iterator );
				}
				else
				{
					CRequestHandler * requestHandler = new CRequestHandler ( medium );
					m_requestHandlers.insert( requestHandler );
					requestHandelers.push_back( requestHandler );
				}
			}
			return requestHandelers;
		}

		iterator++;
	}
	return std::list< CRequestHandler * >();
}

void
CActionHandler::shutDown()
{
}

void
CActionHandler::loop()
{
	std::set< CRequestHandler * > requestHandlersToExecute;
	std::set< CRequestHandler * > requestHandlersToRead;
	while(1)
	{

		{
			boost::lock_guard<boost::mutex> lock( m_mutex );
			std::list< CAction * > toErase;
			BOOST_FOREACH(CAction * action, m_actions)
			{
				std::vector< CRequest* > requests = action->getRequests();

				if ( action->needToExit() && ( action->getDroppedRequests().empty() && action->getRequests().empty() ) )
				{
					toErase.push_back( action );

					if ( action->autoDelete() )
					{
						std::vector< CRequest* > combined = action->getDroppedRequests();

											combined.insert( combined.end(), requests.begin(), requests.end() );
						BOOST_FOREACH( CRequest* request,combined )
						{
							typename RequestToHandlers::iterator lower = m_currentlyUsedHandlers.lower_bound ( request );
							typename RequestToHandlers::iterator upper = m_currentlyUsedHandlers.upper_bound ( request );

							for ( typename RequestToHandlers::iterator it = lower; it!=upper; ++it)
							{
								it->second->deleteRequest( request );
							}
							m_currentlyUsedHandlers.erase( request );
						}

						delete action;
					}
					else
						action->setExecuted();
				}
				else if ( action->requestToProcess() )
				{
					std::vector< CRequest* > combined;

					combined = action->getDroppedRequests();

					combined.insert( combined.end(), requests.begin(), requests.end() );

					BOOST_FOREACH( CRequest* request, requests )
					{
						m_reqToAction.insert( std::make_pair( request, action ) );
					}

					BOOST_FOREACH( CRequest* request, combined )
					{
						typename RequestToHandlers::iterator lower = m_currentlyUsedHandlers.lower_bound (request);
						typename RequestToHandlers::iterator upper = m_currentlyUsedHandlers.upper_bound (request);

						if ( lower == upper )
						{
							std::list< CRequestHandler * > requestHandlers = provideHandler( *request->getMediumFilter() );

							BOOST_FOREACH( CRequestHandler * requestHandler, requestHandlers )
							{
								requestHandlersToExecute.insert( requestHandler );
								requestHandler->setRequest( request );

								m_currentlyUsedHandlers.insert( std::make_pair( request, requestHandler ) );
							}

						}

					}
				}
			}
			BOOST_FOREACH( CAction * action, toErase )
			{
				m_actions.erase( action );
			}
		}

		BOOST_FOREACH( CRequestHandler * reqHandler, requestHandlersToRead )
		{
			BOOST_FOREACH( CAction * action,m_actions )
			{
				std::list< DimsResponse > responses = reqHandler->getDirectActionResponse( action );
				BOOST_FOREACH( DimsResponse const & response, responses )
				{
					CSetResponseVisitor visitor( response );
					action->accept( visitor );
					m_actions.insert( action );
				}
			}
		}

		BOOST_FOREACH( CRequestHandler * reqHandler, requestHandlersToRead )
		{
			reqHandler->processMediumResponses();
		}

		std::multimap< CAction *, CRequest * > eraseCandidates;

		BOOST_FOREACH( typename RequestToAction::value_type & reqAction, m_reqToAction)
		{
			typename RequestToHandlers::iterator lower = m_currentlyUsedHandlers.lower_bound (reqAction.first);
			typename RequestToHandlers::iterator upper = m_currentlyUsedHandlers.upper_bound (reqAction.first);
			for ( typename RequestToHandlers::iterator it = lower; it!=upper; ++it)
			{
				if ( it->second->isProcessed( reqAction.first ) )
				{
					DimsResponse response;

					if ( it->second->getLastResponse( reqAction.first, response ) )
					{
							CSetResponseVisitor visitor( response );
							reqAction.second->accept( visitor );

							it->second->clearLastResponse( reqAction.first );
					}
				}
				else
				{
					// problem ??? assert this??
				}
			}
		}
		m_reqToAction.clear();

		if ( m_actions.empty() )
			boost::this_thread::interruption_point();

		BOOST_FOREACH( CRequestHandler * reqHandler, requestHandlersToExecute )
		{
			reqHandler->runRequests();
			requestHandlersToRead.insert( reqHandler );
		}

		requestHandlersToExecute.clear();
		MilliSleep( m_sleepTime );
	}
}

}
