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

	CScheduleAbleAction (uint256 const & _actionKey );

	virtual void reset();

	void setResult( ScheduledResult const & _result );

	~CScheduleAbleAction();
protected:
	ScheduledResult m_result;
};

}





#endif // SCHEDULE_ABLE_ACTION_H
