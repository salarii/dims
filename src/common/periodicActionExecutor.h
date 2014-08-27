#ifndef PERIODIC_ACTION_EXECUTOR_H
#define PERIODIC_ACTION_EXECUTOR_H

#include "common/action.h"
#include "common/actionHandler.h"
#include "util.h"

namespace common
{

template < class _RequestResponses >
class CDefferedAction
{
public:
	CDefferedAction( CAction< _RequestResponses >* _action, int64_t _deffer );

	bool isReady();

	void execute();
private:
	int64_t const m_deffer;
	int64_t m_time;
	CAction< _RequestResponses >* m_action;
};

template < class _RequestResponses >
CDefferedAction< _RequestResponses >::CDefferedAction( CAction< _RequestResponses >* _action, int64_t _deffer )
	: m_deffer( _deffer )
	, m_time( GetTimeMillis() - m_deffer )
	, m_action( _action )
{
}


template < class _RequestResponses >
bool
CDefferedAction< _RequestResponses >::isReady()
{

	if ( m_action->isExecuted() )
	{
		m_time = GetTimeMillis();
		m_action->reset();
	}

	return GetTimeMillis() - m_time < m_deffer ? false : true;
}

template < class _RequestResponses >
void
CDefferedAction< _RequestResponses >::execute()
{
	CActionHandler< _RequestResponses >::getInstance()->executeAction( m_action );
}


template < class _RequestResponses >
class CPeriodicActionExecutor
{
public:
	static CPeriodicActionExecutor* getInstance();

	~CPeriodicActionExecutor(){};

	void processingLoop();

	void addAction( CAction< _RequestResponses > * _action, unsigned int _milisec );
private:
	CPeriodicActionExecutor();
private:
	static CPeriodicActionExecutor * ms_instance;

	static unsigned int const m_sleepTime;

	mutable boost::mutex m_mutex;

	std::list< CDefferedAction< _RequestResponses > > m_periodicActions;
};

template < class _RequestResponses >
CPeriodicActionExecutor< _RequestResponses >::CPeriodicActionExecutor()
{
}

template < class _RequestResponses >
CPeriodicActionExecutor< _RequestResponses >*
CPeriodicActionExecutor< _RequestResponses >::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CPeriodicActionExecutor();
	};
	return ms_instance;
}

template < class _RequestResponses >
void
CPeriodicActionExecutor< _RequestResponses >::addAction( CAction< _RequestResponses > * _action, unsigned int _milisec )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_periodicActions.push_back( CDefferedAction< _RequestResponses >( _action, _milisec ) );
}

template < class _RequestResponses >
void
CPeriodicActionExecutor< _RequestResponses >::processingLoop()
{

	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock( m_mutex );
			BOOST_FOREACH( CDefferedAction< _RequestResponses > & action, m_periodicActions)
			{
				if ( action.isReady() )
					action.execute();
			}
		}
		MilliSleep(m_sleepTime );
	}

}



}


#endif // PERIODIC_ACTION_EXECUTOR_H
