#ifndef TIME_MEDIUM_H
#define TIME_MEDIUM_H

#include "common/medium.h"
#include "common/mediumRequests.h"
#include "common/actionHandler.h"

namespace common
{

// this  may be  not  so precise in terms of  time measure
// more or less result is good enough
template < class Medium >
class CTimeMedium : public Medium
{
public:
	typedef TYPE(Medium) Types;
	typedef RESPONSE_TYPE(Types) Response;
public:
	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::multimap< CRequest< Types > const*, Response > & _requestResponse );

	void add( CTimeEventRequest< Types > const * _request );

	void setResponse( Response const & _responses );

	void deleteRequest( CRequest< Types >const* _request );

	void workLoop();

	static CTimeMedium* getInstance();
protected:
	//for now  hardcode
	CTimeMedium():m_sleepTime( 100 ){};

	void clearResponses();
protected:
	mutable boost::mutex m_mutex;

	std::multimap< common::CRequest< Types >const*, Response > m_responses;

	std::map< CTimeEventRequest< Types > const *, int64_t > m_timeLeftToTrigger;

	static CTimeMedium* ms_instance;

	int64_t const m_sleepTime;
};

template < class Medium >
CTimeMedium< Medium >*
CTimeMedium< Medium >::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CTimeMedium< Medium >();
	};
	return ms_instance;
}

template < class Medium >
bool
CTimeMedium< Medium >::serviced() const
{
	return !m_responses.empty();
}


template < class Medium >
bool
CTimeMedium< Medium >::flush()
{
	return true;
}

template < class Medium >
bool
CTimeMedium< Medium >::getResponseAndClear( std::multimap< CRequest< Types >const*, Response > & _requestResponse )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	_requestResponse = m_responses;

	clearResponses();
	return true;
}

template < class Medium >
void
CTimeMedium< Medium >::clearResponses()
{
	m_responses.clear();
}

template < class Medium >
void
CTimeMedium< Medium >::add( CTimeEventRequest< Types > const * _request )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_timeLeftToTrigger.insert( std::make_pair( _request, _request->getEventTime() ) );
}

template < class Medium >
void
CTimeMedium< Medium >::deleteRequest( CRequest< Types >const* _request )
{
	m_timeLeftToTrigger.erase( ( CTimeEventRequest< Types > const * )_request );// ugly  solution
		m_responses.erase( _request );
}

template < class Medium >
void
CTimeMedium< Medium >::workLoop()
{
	while(1)
	{
		MilliSleep( m_sleepTime );
		boost::lock_guard<boost::mutex> lock( m_mutex );

		std::vector< CTimeEventRequest< Types > const * > toTrigger;

		typename std::map< CTimeEventRequest< Types > const *, int64_t >::iterator iterator = m_timeLeftToTrigger.begin();
		while( iterator != m_timeLeftToTrigger.end() )
		{
			if ( iterator->second - m_sleepTime <= 0 )
			{
				toTrigger.push_back( iterator->first );
			}
			else
			{
				iterator->second -= m_sleepTime;
			}
			iterator++;
		}

		BOOST_FOREACH( CTimeEventRequest< Types > const * timeEventReq, toTrigger )
		{
			m_timeLeftToTrigger.erase( timeEventReq );
			m_responses.insert( std::make_pair( timeEventReq, CTimeEvent() ) );
		}
	}

}

}
#endif // TIME_MEDIUM_H
