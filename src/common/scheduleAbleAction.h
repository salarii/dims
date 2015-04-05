#ifndef SCHEDULE_ABLE_ACTION_H
#define SCHEDULE_ABLE_ACTION_H

#include "common/action.h"
#include "common/scheduledActionManager.h"

namespace common
{

template < class _Type >
class CScheduleAbleAction : public CAction< _Type >
{
public:
		CScheduleAbleAction ()
		{
			m_scheduleKey = getRandNumber();
		}

		uint256 getScheduleKey() const
		{
			return m_scheduleKey;
		}

		virtual void reset()
		{
			CScheduledActionManager< _Type >::getInstance()->setResponseForAction( m_scheduleKey, m_result );
			CAction< _Type >::reset();
		}

		~CScheduleAbleAction()
		{
			CScheduledActionManager< _Type >::getInstance()->setResponseForAction( m_scheduleKey, m_result );
		}
	protected:
		uint256 m_scheduleKey;

		ScheduledResult m_result;
};

}





#endif // SCHEDULE_ABLE_ACTION_H
