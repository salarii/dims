// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/scheduledActionManager.h"

namespace common
{

common::CScheduledActionManager * common::CScheduledActionManager::ms_instance = NULL;

CScheduledActionManager *
CScheduledActionManager::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CScheduledActionManager ();
	};
	return ms_instance;
}

bool
CScheduledActionManager::serviced() const
{
	return !m_responses.empty();
}

void
CScheduledActionManager::deleteRequest( CRequest const* _request )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_responses.erase( _request );

	typename std::multimap< uint256, CRequest const* >::iterator iterator = m_actionToRequest.begin();

	while( iterator != m_actionToRequest.end() )
	{
		if ( iterator->second == _request )
		{
			m_actionToRequest.erase( iterator );
			break;
		}

		++iterator;
	}
}

bool
CScheduledActionManager::flush()
{
	return true;
}

bool
CScheduledActionManager::getResponseAndClear( std::multimap< CRequest const*, DimsResponse > & _requestResponse )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	_requestResponse = m_responses;
	clearResponses();
	return true;
}

void
CScheduledActionManager::clearResponses()
{
	m_responses.clear();
}

void
CScheduledActionManager::setResponseForAction( ScheduledResult const & _responses, uint256 const & _id )
{
		boost::lock_guard<boost::mutex> lock( m_mutex );
		typename std::multimap< uint256, CRequest const* >::const_iterator iterator = m_actionToRequest.find( _id );

		if( iterator == m_actionToRequest.end() )
			return;

	m_responses.insert( std::make_pair( iterator->second, _responses ) );
}

void
CScheduledActionManager::add( CScheduleActionRequest const * _request )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_actionToRequest.insert( std::make_pair( _request->getAction()->getActionKey(), _request ) );

	_request->getAction()->registerSetResponseWhileDeleted( boost::bind( &CScheduledActionManager::setResponseForAction, this, _1, _2 ) );

	common::CActionHandler ::getInstance()->executeAction( _request->getAction() );
}

}
