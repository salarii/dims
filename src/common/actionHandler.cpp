// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "actionHandler.h"
#include "setResponseVisitor.h"
#include "request.h"

#include <exception> 
#include <boost/foreach.hpp>

#include "util.h"

namespace common
{

unsigned int const CActionHandler::m_sleepTime = 2000;

CActionHandler * CActionHandler::ms_instance = NULL;


void
CAction::accept( CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

CActionHandler::CActionHandler()
{
}

CActionHandler::~CActionHandler()
{
}

CActionHandler*
CActionHandler::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CActionHandler();
	};
	return ms_instance;
}

void
CActionHandler::executeAction( CAction* _action )
{
	boost::lock_guard<boost::mutex> lock(m_mutex);
	m_actions.push_back( _action );
}

void
CActionHandler::addConnectionProvider( CConnectionProvider* _connectionProvider )
{
	m_connectionProviders.push_back( _connectionProvider );
}

std::list< CRequestHandler * >
CActionHandler::provideHandler( int const _requestKind )
{
	std::list< CRequestHandler * > requestHandelers;

	{
		std::pair< std::map<int, CRequestHandler * >::iterator, std::map<int, CRequestHandler * >::iterator > range;

		 range = m_requestHandlers.equal_range( _requestKind );

		 for ( std::map<int, CRequestHandler * >::iterator it = range.first; it != range.second; ++it )
			 requestHandelers.push_back( it->second );

		if ( range.first != m_requestHandlers.end() )
			return requestHandelers;
	}

	std::list<CConnectionProvider*>::iterator iterator = m_connectionProviders.begin();

	while( iterator != m_connectionProviders.end() )
	{
		std::list< CMedium *> mediums= (*iterator)->provideConnection( _requestKind );

		if ( !mediums.empty() )
		{
			BOOST_FOREACH( CMedium * medium, mediums )
			{
				CRequestHandler * requestHandler = new CRequestHandler( medium );
				m_requestHandlers.insert( std::make_pair( _requestKind, requestHandler ) );
				requestHandelers.push_back( requestHandler );

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
	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock( m_mutex );
			BOOST_FOREACH(CAction* action, m_actions)
			{
				CRequest* request = action->execute();

				if ( request )
					m_reqToAction.insert( std::make_pair( request, action ) );
			}
			m_actions.clear();
		}

		BOOST_FOREACH( AvailableHandlers::value_type & reqAction, m_requestHandlers)
		{
			reqAction.second->readLoop();
		}

		std::list< CRequest* > requestsToErase;

		BOOST_FOREACH(RequestToAction::value_type & reqAction, m_reqToAction)
		{
			std::list< CRequestHandler * > requestHandlers = provideHandler( reqAction.first->getKind() );

			std::list< RequestResponse > responses;
			BOOST_FOREACH( CRequestHandler * requestHandler, requestHandlers )
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

				CSetResponseVisitor visitor( responses );
				reqAction.second->accept( visitor );

				m_actions.push_back( reqAction.second );
		}

		BOOST_FOREACH( CRequest* & request, requestsToErase)
		{
			m_reqToAction.erase( request );
		}

		if ( m_reqToAction.empty() )
			boost::this_thread::interruption_point();

		BOOST_FOREACH( AvailableHandlers::value_type & reqAction, m_requestHandlers)
		{
			reqAction.second->runRequests();
		}

        MilliSleep( m_sleepTime );
	}
}


}
