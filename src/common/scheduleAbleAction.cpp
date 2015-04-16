#include "common/action.h"
#include "common/scheduledActionManager.h"
#include "common/scheduleAbleAction.h"

namespace common
{

template < class _Type >
CScheduleAbleAction< _Type >::CScheduleAbleAction()
{}

template < class _Type >
CScheduleAbleAction< _Type >::CScheduleAbleAction(uint256 const & _actionKey )
	: CAction< _Type >( _actionKey )
{
}

template < class _Type >
void
CScheduleAbleAction< _Type >::reset()
{
	CScheduledActionManager< _Type >::getInstance()->setResponseForAction( CAction< _Type >::m_actionKey, m_result );
	CAction< _Type >::reset();
}

template < class _Type >
void
CScheduleAbleAction< _Type >::setResult( ScheduledResult const & _result )
{
	m_result = _result;
}

template < class _Type >
CScheduleAbleAction< _Type >::~CScheduleAbleAction()
{
	CScheduledActionManager< _Type >::getInstance()->setResponseForAction( CAction< _Type >::m_actionKey, m_result );
}
//ugly
template class CScheduleAbleAction< CTrackerTypes >;
template class CScheduleAbleAction< CMonitorTypes >;

template<>
CScheduledActionManager< CMonitorTypes > * CScheduledActionManager< CMonitorTypes >::ms_instance = NULL;

template<>
CScheduledActionManager< CTrackerTypes > * CScheduledActionManager< CTrackerTypes >::ms_instance = NULL;

}

