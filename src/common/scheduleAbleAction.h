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
			m_actionKey = getRandNumber();
		}

		uint256 getScheduleKey() const
		{
			return m_actionKey;
		}

		virtual void reset()
		{

			CScheduledActionManager:
			CAction< _Type >::reset();
		}

		~CScheduleAbleAction()
		{}
	protected:
		uint256 m_scheduleKey;

		ScheduledResult m_result;
};

}





#endif // SCHEDULE_ABLE_ACTION_H
