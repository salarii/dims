#ifndef TIME_MEDIUM_H
#define TIME_MEDIUM_H

#include "common/medium.h"
#include "common/mediumRequests.h"
#include "common/actionHandler.h"

namespace common
{

// this  may be  not  so precise in terms of  time measure
// more or less result is good enough
template < class ResponseType >
class CTimeMedium : public common::CMedium< ResponseType >
{
public:
	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::multimap< CRequest< ResponseType >const*, ResponseType > & _requestResponse );

	void add( CTimeEventRequest< ResponseType > const * _request );

	void setResponse( ResponseType const & _responses );

	void workLoop();

	static CTimeMedium* getInstance();
protected:
	//for now  hardcode
	CTimeMedium():m_sleepTime( 100 ){};

	void clearResponses();
protected:
	mutable boost::mutex m_mutex;

	std::multimap< common::CRequest< ResponseType >const*, ResponseType > m_responses;

	std::map< CTimeEventRequest< ResponseType > const *, int64_t > m_timeLeftToTrigger;

	static CTimeMedium* ms_instance;

	int64_t const m_sleepTime;
};

template < class ResponseType >
CTimeMedium< ResponseType >*
CTimeMedium< ResponseType >::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CTimeMedium< ResponseType >();
	};
	return ms_instance;
}

template < class ResponseType >
bool
CTimeMedium< ResponseType >::serviced() const
{
	return !m_responses.empty();
}


template < class ResponseType >
bool
CTimeMedium< ResponseType >::flush()
{
	return true;
}

template < class ResponseType >
bool
CTimeMedium< ResponseType >::getResponseAndClear( std::multimap< CRequest< ResponseType >const*, ResponseType > & _requestResponse )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	_requestResponse = m_responses;

	clearResponses();
	return true;
}

template < class ResponseType >
void
CTimeMedium< ResponseType >::clearResponses()
{
	m_responses.clear();
}

template < class ResponseType >
void
CTimeMedium< ResponseType >::add( CTimeEventRequest< ResponseType > const * _request )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_timeLeftToTrigger.insert( std::make_pair( _request, _request->getEventTime() ) );
}

template < class ResponseType >
void
CTimeMedium< ResponseType >::workLoop()
{
	while(1)
	{
		MilliSleep( m_sleepTime );
		boost::lock_guard<boost::mutex> lock( m_mutex );

		std::vector< CTimeEventRequest< ResponseType > const * > toTrigger;

		typename std::map< CTimeEventRequest< ResponseType > const *, int64_t >::iterator iterator = m_timeLeftToTrigger.begin();
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

		BOOST_FOREACH( CTimeEventRequest< ResponseType > const * timeEventReq, toTrigger )
		{
			m_timeLeftToTrigger.erase( timeEventReq );
			m_responses.insert( std::make_pair( timeEventReq, CTimeEvent() ) );
		}
	}

}

}
#endif // TIME_MEDIUM_H
