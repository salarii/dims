#ifndef TIME_MEDIUM_H
#define TIME_MEDIUM_H

#include "common/medium.h"
#include "common/mediumRequests.h"
#include "common/nodeMedium.h"
#include "common/actionHandler.h"

namespace common
{

template < class ResponseType >
class CTimeMedium : public common::CMedium< ResponseType >
{
public:
	CTimeMedium(){};

	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::map< CRequest< ResponseType >const*, std::vector< ResponseType > > & _requestResponse );

	void add( CTimeEventRequest< ResponseType > const * _request );

	void setResponse( ResponseType const & _responses );

	common::CSelfNode * getNode() const;

	void workLoop();
protected:
	void clearResponses();
protected:
	mutable boost::mutex m_mutex;

	std::map< common::CRequest< ResponseType >const*,  std::vector< ResponseType > > m_responses;

	std::map< CTimeEventRequest< ResponseType > const *, int64_t > m_timeLeftToTrigger;
};

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
CTimeMedium< ResponseType >::getResponseAndClear( std::map< CRequest< ResponseType >*, std::vector< ResponseType > > & _requestResponse )
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
}

}
#endif // TIME_MEDIUM_H
