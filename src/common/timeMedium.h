// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TIME_MEDIUM_H
#define TIME_MEDIUM_H

#include "common/medium.h"
#include "common/requests.h"
#include "common/actionHandler.h"

namespace common
{

// this  may be  not  so precise in terms of  time measure
// more or less result is good enough
class CTimeMedium : public CMedium
{
public:
	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::multimap< CRequest const*, DimsResponse > & _requestResponse );

	void add( CTimeEventRequest const * _request );

	void setResponse( DimsResponse const & _responses );

	void deleteRequest( CRequest const* _request );

	void workLoop();

	static CTimeMedium* getInstance();
protected:
	//for now  hardcode
	CTimeMedium():m_sleepTime( 100 ){};

	void clearResponses();
protected:
	mutable boost::mutex m_mutex;

	std::multimap< common::CRequest const*, DimsResponse > m_responses;

	std::map< CTimeEventRequest const *, int64_t > m_timeLeftToTrigger;

	static CTimeMedium* ms_instance;

	int64_t const m_sleepTime;
};

}
#endif // TIME_MEDIUM_H
