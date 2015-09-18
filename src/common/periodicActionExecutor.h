#ifndef PERIODIC_ACTION_EXECUTOR_H
#define PERIODIC_ACTION_EXECUTOR_H

#include "common/action.h"
#include "common/actionHandler.h"
#include "util.h"

namespace common
{

template < class _Type >
class CDefferedAction
{
public:
	CDefferedAction( CAction * _action, int64_t _deffer );

	bool isReady();

	void reset();

	void getRequest() const;
private:
	int64_t const m_deffer;
	int64_t m_time;
	CAction * m_action;
};

template < class _Type >
CDefferedAction ::CDefferedAction( CAction * _action, int64_t _deffer )
	: m_deffer( _deffer )
	, m_time( GetTimeMillis() - m_deffer )
	, m_action( _action )
{
}


template < class _Type >
bool
CDefferedAction ::isReady()
{

	bool readyToRun = true;
	if ( m_action->isExecuted() )
	{
		readyToRun = GetTimeMillis() - m_time < m_deffer ? false : true;

		if ( readyToRun )
			reset();
	}
	else
	{
		m_time = GetTimeMillis();
	}
	return readyToRun;
}

template < class _Type >
void
CDefferedAction ::reset()
{
	m_action->reset();
}


template < class _Type >
void
CDefferedAction ::getRequest() const
{
	CActionHandler ::getInstance()->executeAction( m_action );
}


template < class _Type >
class CPeriodicActionExecutor
{
public:
	static CPeriodicActionExecutor* getInstance();

	~CPeriodicActionExecutor(){};

	void processingLoop();

	void addAction( CAction * _action, unsigned int _milisec );
private:
	CPeriodicActionExecutor();
private:
	static CPeriodicActionExecutor * ms_instance;

	static unsigned int const m_sleepTime;

	mutable boost::mutex m_mutex;

	std::list< CDefferedAction  > m_periodicActions;
};

template < class _Type >
CPeriodicActionExecutor ::CPeriodicActionExecutor()
{
}

template < class _Type >
CPeriodicActionExecutor *
CPeriodicActionExecutor ::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CPeriodicActionExecutor();
	};
	return ms_instance;
}

template < class _Type >
void
CPeriodicActionExecutor ::addAction( CAction * _action, unsigned int _milisec )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_periodicActions.push_back( CDefferedAction ( _action, _milisec ) );
}

template < class _Type >
void
CPeriodicActionExecutor ::processingLoop()
{

	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock( m_mutex );
			BOOST_FOREACH( CDefferedAction & action, m_periodicActions)
			{
				if ( action.isReady() )
				{
					action.getRequest();
				}
			}
		}
		MilliSleep(m_sleepTime );
	}

}



}


#endif // PERIODIC_ACTION_EXECUTOR_H
