// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_CONNECTION_MANAGER_H
#define NODE_CONNECTION_MANAGER_H

#include <set>
#include <QString>
#include <boost/thread.hpp>

#include "connectionProvider.h"

namespace node
{

class CAction;

class CActionHandler;

class CNodeConnectionManager
{
public:
	~CNodeConnectionManager();

	static CNodeConnectionManager* getInstance();

	void connectToNetwork();

	void periodicActionLoop();

	void executeAction();

	void addPeriodicAction( CAction* _action );

private:
	CNodeConnectionManager();
private:
	static CNodeConnectionManager * ms_instance;

	CActionHandler * m_actionHandler;

	static unsigned int const m_sleepTime;

	boost::mutex m_mutex;

	std::list< CAction * > m_periodicActions;
};


}

#endif // NODE_CONNECTION_MANAGER_H