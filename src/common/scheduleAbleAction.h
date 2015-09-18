#ifndef SCHEDULE_ABLE_ACTION_H
#define SCHEDULE_ABLE_ACTION_H

#include "common/action.h"
#include "common/responses.h"
//need to  do  explicit because  those classes are circularly dependent
#include <boost/signals2.hpp>
namespace common
{

class CScheduleAbleAction : public CAction 
{
public:
	CScheduleAbleAction ();

	CScheduleAbleAction (uint256 const & _actionKey );

	virtual void reset();

	void setResult( ScheduledResult const & _result );

	void registerSetResponseWhileDeleted( boost::signals2::slot< void ( ScheduledResult const &, uint256 const & ) > const & _hook );

	~CScheduleAbleAction();
protected:
	ScheduledResult m_result;

	boost::signals2::signal< void ( ScheduledResult const &, uint256 const & ) > m_setResponse;
};

inline
CScheduleAbleAction ::CScheduleAbleAction()
{}

inline
CScheduleAbleAction ::CScheduleAbleAction(uint256 const & _actionKey )
	: CAction ( _actionKey )
{
}

inline void
CScheduleAbleAction ::reset()
{
	m_setResponse( m_result, CAction ::m_actionKey );
	CAction ::reset();
}

inline void
CScheduleAbleAction ::setResult( ScheduledResult const & _result )
{
	m_result = _result;
}

inline
CScheduleAbleAction ::~CScheduleAbleAction()
{
	m_setResponse( m_result, CAction ::m_actionKey );
}

inline void
CScheduleAbleAction ::registerSetResponseWhileDeleted( boost::signals2::slot< void ( ScheduledResult const &, uint256 const & ) > const & _hook )
{
	m_setResponse.connect( _hook );
}


}





#endif // SCHEDULE_ABLE_ACTION_H
