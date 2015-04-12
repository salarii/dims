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
CScheduleAbleAction< _Type >::~CScheduleAbleAction()
{
	CScheduledActionManager< _Type >::getInstance()->setResponseForAction( CAction< _Type >::m_actionKey, m_result );
}

template class CScheduleAbleAction< CTrackerTypes >;


}


