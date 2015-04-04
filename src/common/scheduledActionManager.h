#ifndef SCHEDULED_ACTION_MANAGER_H
#define SCHEDULED_ACTION_MANAGER_H

#include "common/request.h"

namespace common
{

template < class _Type >
class CScheduledActionManager : public _Type::Medium
{
public:
	static CScheduledActionManager * getInstance();

	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::multimap< CRequest< Type >const*, _Type::Response > & _requestResponse );

	void setResponse( uint256 const & _id, Response const & _responses );

	void setResponseForAction( uint256 const & _id, ScheduledResult const & _responses );
protected:
	CScheduledActionManager(){};
protected:
	static CScheduledActionManager< _Type > * ms_instance;
	// scheduleable action result as  boost  variant ??
	// but  this  have  to  be  passed  through
	std::multimap< CRequest< Type >const*, Response > m_responses;
	std::multimap< uint256, uint256 > m_actionToRequest;
};

static CScheduledActionManager * getInstance();
CScheduledActionManager * CScheduledActionManager::ms_instance = NULL;

CScheduledActionManager*
CScheduledActionManager::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CScheduledActionManager();
	};
	return ms_instance;
}

template < class _Medium >
bool
CScheduledActionManager< _Medium >::serviced() const
{
	return false;
}

template < class _Medium >
void
CScheduledActionManager< _Medium >::updateLastRequest( uint256 const & _id, common::CRequest< Type >const* _request )
{
}

template < class _Medium >
bool
CScheduledActionManager< _Medium >::flush()
{
	return true;
}

template < class _Medium >
bool
CScheduledActionManager< _Medium >::getResponseAndClear( std::multimap< CRequest< Type >const*, RESPONSE_TYPE(_Medium) > & _requestResponse )
{
	return true;
}

template < class _Medium >
void
CScheduledActionManager< _Medium >::clearResponses()
{
}

template < class _Medium >
void
CScheduledActionManager< _Medium >::setResponse( uint256 const & _id, Response const & _response )
{
}

}
#endif // SCHEDULED_ACTION_MANAGER_H
