#ifndef SCHEDULE_ABLE_ACTION_H
#define SCHEDULE_ABLE_ACTION_H

#include "common/action.h"
#include "common/commonResponses.h"
//need to  do  explicit because  those classes are circularly dependent
#include <boost/signals2.hpp>
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

	void registerSetResponseWhileDeleted( boost::signals2::slot< void ( ScheduledResult const &, uint256 const & ) > const & _hook );

	~CScheduleAbleAction();
protected:
	ScheduledResult m_result;

	boost::signals2::signal< void ( ScheduledResult const &, uint256 const & ) > m_setResponse;
};

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
	m_setResponse( m_result, CAction< _Type >::m_actionKey );
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
	m_setResponse( m_result, CAction< _Type >::m_actionKey );
}

template < class _Type >
void
CScheduleAbleAction< _Type >::registerSetResponseWhileDeleted( boost::signals2::slot< void ( ScheduledResult const &, uint256 const & ) > const & _hook )
{
	m_setResponse.connect( _hook );
}


}





#endif // SCHEDULE_ABLE_ACTION_H
