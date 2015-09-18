// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/periodicActionExecutor.h"

namespace common
{

CDefferedAction ::CDefferedAction( CAction * _action, int64_t _deffer )
	: m_deffer( _deffer )
	, m_time( GetTimeMillis() - m_deffer )
	, m_action( _action )
{
}

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

void
CDefferedAction ::reset()
{
	m_action->reset();
}

void
CDefferedAction ::getRequest() const
{
	CActionHandler ::getInstance()->executeAction( m_action );
}

CPeriodicActionExecutor * CPeriodicActionExecutor::ms_instance = NULL;

unsigned int const CPeriodicActionExecutor::m_sleepTime = 100;

CPeriodicActionExecutor ::CPeriodicActionExecutor()
{
}

CPeriodicActionExecutor *
CPeriodicActionExecutor ::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CPeriodicActionExecutor();
	};
	return ms_instance;
}

void
CPeriodicActionExecutor ::addAction( CAction * _action, unsigned int _milisec )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );
	m_periodicActions.push_back( CDefferedAction ( _action, _milisec ) );
}

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
