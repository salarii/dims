#ifndef SCHEDULE_ABLE_ACTION_H
#define SCHEDULE_ABLE_ACTION_H

#include "common/action.h"

//need to  do  explicit because  those classes are circularly dependent

namespace common
{

template < class _Type >
class CScheduleAbleAction : public CAction< _Type >
{
public:
		CScheduleAbleAction ();

		uint256 getScheduleKey() const;

		virtual void reset();

		~CScheduleAbleAction();
	protected:
		uint256 m_scheduleKey;

		ScheduledResult m_result;
};

}





#endif // SCHEDULE_ABLE_ACTION_H
