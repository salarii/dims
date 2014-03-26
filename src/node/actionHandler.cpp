// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "actionHandler.h"
#include <exception> 

#include "setResponseVisitor.h"

#include <boost/foreach.hpp>


namespace node
{

unsigned int const CActionHandle::m_sleepTime = 2;

self::TransactionsStatus::Enum m_status;

uint256 m_token;


void
CAction::accept( CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

typedef std::pair<node::CRequest* const, node::CAction*> ReqAction;

void 
CActionHandle::run()
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
			if ( m_requestHandler->isProcessed( reqAction.first ) )
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