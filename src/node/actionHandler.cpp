// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "actionHandler.h"
#include <exception> 

#include "setResponseVisitor.h"

#include <boost/foreach.hpp>


namespace node
{

unsigned int const CActionHandler::m_sleepTime = 2;

self::TransactionsStatus::Enum m_status;

uint256 m_token;

typedef std::pair<node::CRequest* const, node::CAction*> ReqAction;

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
		CNetworkClient * networkClient = (*iterator)->provideConnection( _requestKind );

		if ( networkClient != NULL )
		{
			CRequestHandler * requestHandler = new CRequestHandler( networkClient );
			m_requestHandlers.insert( std::make_pair( _requestKind, requestHandler ) );
			return requestHandler;
		}
	}
	return NULL;
}

void
CActionHandler::shutDown()
{

}

void 
CActionHandler::run()
{
	while(1)
	{
		{
			QMutexLocker lock( &m_mutex );
			BOOST_FOREACH(CAction* action, m_actions)
			{
				CRequest* request = action->execute();

				if ( request )
					m_reqToAction.insert( std::make_pair( request, action ) );
			}

			m_actions.clear();

		}

		BOOST_FOREACH(ReqAction & reqAction, m_reqToAction)
		{
			if ( provideHandler( reqAction.first->getKind() )->isProcessed( reqAction.first ) )
			{
				CSetResponseVisitor visitor( m_requestHandler->getRespond( reqAction.first ) );
				reqAction.second->accept( visitor );
				
				m_actions.push_back( reqAction.second );
			}
		}
		QThread::sleep ( m_sleepTime );
	}
}

}
