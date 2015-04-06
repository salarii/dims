#include "common/action.h"
#include "common/scheduledActionManager.h"
#include "common/scheduleAbleAction.h"

namespace common
{

template < class _Type >
CScheduleAbleAction< _Type >::CScheduleAbleAction()
{}

template < class _Type >
uint256
CScheduleAbleAction< _Type >::getScheduleKey() const
{
	return m_scheduleKey;
}

template < class _Type >
void
CScheduleAbleAction< _Type >::reset()
{
	CScheduledActionManager< _Type >::getInstance()->setResponseForAction( m_scheduleKey, m_result );
	CAction< _Type >::reset();
}

template < class _Type >
CScheduleAbleAction< _Type >::~CScheduleAbleAction()
{
	CScheduledActionManager< _Type >::getInstance()->setResponseForAction( m_scheduleKey, m_result );
}

template class CScheduleAbleAction< CTrackerTypes >;


}


