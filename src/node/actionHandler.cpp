// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "actionHandler.h"
#include "setResponseVisitor.h"
#include "networkClient.h"
#include "request.h"

#include <exception> 
#include <boost/foreach.hpp>

namespace node
{

unsigned int const CActionHandler::m_sleepTime = 2;

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

CRequestHandler * 
CActionHandler::provideHandler( RequestKind::Enum const _requestKind )
{
	{
		std::map<RequestKind::Enum, CRequestHandler * >::iterator iterator =  m_requestHandlers.find( _requestKind );

		if ( iterator != m_requestHandlers.end() )
			return iterator->second;
	}

	std::list<CConnectionProvider*>::iterator iterator = m_connectionProviders.begin();

	while( iterator != m_connectionProviders.end() )
	{
		CMedium * medium = (*iterator)->provideConnection( _requestKind );
		if ( medium != NULL )
		{
			CRequestHandler * requestHandler = new CRequestHandler( medium );
			m_requestHandlers.insert( std::make_pair( _requestKind, requestHandler ) );
			return requestHandler;
		}
        iterator++;
	}
	return NULL;
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
			CRequestHandler * requestHandler = provideHandler( reqAction.first->getKind() );
			if ( requestHandler )
			{
				if ( requestHandler->isProcessed( reqAction.first ) )
				{
					CSetResponseVisitor visitor( requestHandler->getRespond( reqAction.first ) );
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

		QThread::sleep ( m_sleepTime );
	}
}

}
