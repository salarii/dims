// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/timeMedium.h"

namespace common
{

common::CTimeMedium * common::CTimeMedium::ms_instance = NULL;

CTimeMedium*
CTimeMedium::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CTimeMedium();
	};
	return ms_instance;
}

bool
CTimeMedium::serviced() const
{
	return !m_responses.empty();
}

bool
CTimeMedium::flush()
{
	return true;
}

bool
CTimeMedium::getResponseAndClear( std::multimap< CRequest const*, DimsResponse > & _requestResponse )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	_requestResponse = m_responses;

	clearResponses();
	return true;
}

void
CTimeMedium::clearResponses()
{
	m_responses.clear();
}

void
CTimeMedium::add( CTimeEventRequest const * _request )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_timeLeftToTrigger.insert( std::make_pair( _request, _request->getEventTime() ) );
}

void
CTimeMedium::deleteRequest( CRequest const* _request )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_timeLeftToTrigger.erase( ( CTimeEventRequest const * )_request );// ugly  solution
		m_responses.erase( _request );
}

void
CTimeMedium::workLoop()
{
	while(1)
	{
		MilliSleep( m_sleepTime );
		boost::lock_guard<boost::mutex> lock( m_mutex );

		std::vector< CTimeEventRequest const * > toTrigger;

		typename std::map< CTimeEventRequest const *, int64_t >::iterator iterator = m_timeLeftToTrigger.begin();
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

		BOOST_FOREACH( CTimeEventRequest const * timeEventReq, toTrigger )
		{
			m_timeLeftToTrigger.erase( timeEventReq );
			m_responses.insert( std::make_pair( timeEventReq, CTimeEvent() ) );
		}

		if ( m_timeLeftToTrigger.empty() && m_responses.empty() )
			boost::this_thread::interruption_point();

	}

}


}
