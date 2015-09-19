// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PERIODIC_ACTION_EXECUTOR_H
#define PERIODIC_ACTION_EXECUTOR_H

#include "util.h"

namespace common
{

class CAction;

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

}
#endif // PERIODIC_ACTION_EXECUTOR_H
