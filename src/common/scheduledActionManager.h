#ifndef SCHEDULED_ACTION_MANAGER_H
#define SCHEDULED_ACTION_MANAGER_H

#include "common/request.h"
#include "common/medium.h"
#include "common/commonRequests.h"
#include "common/actionHandler.h"
#include "common/scheduleAbleAction.h"

namespace common
{

template < class _Type >
class CScheduledActionManager : public _Type::Medium
{
public:
	static CScheduledActionManager * getInstance();

	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::multimap< CRequest< _Type >const*, typename _Type::Response > & _requestResponse );

	void setResponseForAction( uint256 const & _id, ScheduledResult const & _responses );

	void add( CScheduleActionRequest< _Type > const * _request );

	void clearResponses();

	void deleteRequest( CRequest< _Type >const* _request );
protected:
	CScheduledActionManager(){};
protected:
	mutable boost::mutex m_mutex;

	static CScheduledActionManager< _Type > * ms_instance;
	// scheduleable action result as  boost  variant ??
	// but  this  have  to  be  passed  through
	std::multimap< CRequest< _Type >const*, typename _Type::Response > m_responses;

	std::multimap< uint256, CRequest< _Type >const* > m_actionToRequest;
};

template < class _Type >
CScheduledActionManager< _Type >*
CScheduledActionManager< _Type >::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CScheduledActionManager< _Type >();
	};
	return ms_instance;
}

template < class _Type >
bool
CScheduledActionManager< _Type >::serviced() const
{
	return !m_responses.empty();
}

template < class _Type >
void
CScheduledActionManager< _Type >::deleteRequest( CRequest< _Type >const* _request )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_responses.erase( _request );

	typename std::multimap< uint256, CRequest< _Type >const* >::iterator iterator = m_actionToRequest.begin();

	while( iterator != m_actionToRequest.end() )
	{
		if ( iterator->second == _request )
		{
			m_actionToRequest.erase( iterator );
			break;
		}
	}
}

template < class _Type >
bool
CScheduledActionManager< _Type >::flush()
{
	return true;
}

template < class _Type >
bool
CScheduledActionManager< _Type >::getResponseAndClear( std::multimap< CRequest< _Type >const*, typename _Type::Response > & _requestResponse )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	_requestResponse = m_responses;
	clearResponses();
	return true;
}

template < class _Type >
void
CScheduledActionManager< _Type >::clearResponses()
{
	m_responses.clear();
}

template < class _Type >
void
CScheduledActionManager< _Type >::setResponseForAction( uint256 const & _id, ScheduledResult const & _responses )
{
		boost::lock_guard<boost::mutex> lock( m_mutex );
		typename std::multimap< uint256, CRequest< _Type >const* >::const_iterator iterator = m_actionToRequest.find( _id );

		if( iterator != m_actionToRequest.end() )
			return;

	m_responses.insert( std::make_pair( iterator->second, _responses ) );
}

template < class _Type >
void
CScheduledActionManager< _Type >::add( CScheduleActionRequest< _Type > const * _request )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_actionToRequest.insert( std::make_pair( _request->getAction()->getActionKey(), _request ) );
	common::CActionHandler< _Type >::getInstance()->executeAction( _request->getAction() );
}

}
#endif // SCHEDULED_ACTION_MANAGER_H
