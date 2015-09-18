// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SCHEDULED_ACTION_MANAGER_H
#define SCHEDULED_ACTION_MANAGER_H

#include "common/request.h"
#include "common/medium.h"
#include "common/requests.h"
#include "common/actionHandler.h"
#include "common/scheduleAbleAction.h"

namespace common
{

class CScheduledActionManager : public CMedium
{
public:
	static CScheduledActionManager * getInstance();

	bool serviced() const;

	bool flush();

	bool getResponseAndClear( std::multimap< CRequest const*, DimsResponse > & _requestResponse );

	void setResponseForAction( ScheduledResult const & _responses, uint256 const & _id );

	void add( CScheduleActionRequest const * _request );

	void clearResponses();

	void deleteRequest( CRequest const* _request );
protected:
	CScheduledActionManager(){};
protected:
	mutable boost::mutex m_mutex;

	static CScheduledActionManager * ms_instance;
	// scheduleable action result as  boost  variant ??
	// but  this  have  to  be  passed  through
	std::multimap< CRequest const*, DimsResponse > m_responses;

	std::multimap< uint256, CRequest const* > m_actionToRequest;
};

}
#endif // SCHEDULED_ACTION_MANAGER_H
