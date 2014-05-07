// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_CONNECTION_MANAGER_H
#define NODE_CONNECTION_MANAGER_H

#include <set>
#include <QString>
#include <boost/thread.hpp>


#include "configureNodeActionHadler.h"

namespace common
{

template < class _RequestResponses >
class CActionHandler;

template < class _RequestResponses >
class CAction;
}

namespace node
{

class CNodeConnectionManager
{
public:
	~CNodeConnectionManager();

	static CNodeConnectionManager* getInstance();

	void connectToNetwork();

	void periodicActionLoop();

	void executeAction();

	void addPeriodicAction( common::CAction< NodeResponses >* _action );

private:
	CNodeConnectionManager();
private:
	static CNodeConnectionManager * ms_instance;

	common::CActionHandler< NodeResponses > * m_actionHandler;

	static unsigned int const m_sleepTime;

    mutable boost::mutex m_mutex;

	std::list< common::CAction< NodeResponses > * > m_periodicActions;
};


}

#endif // NODE_CONNECTION_MANAGER_H
