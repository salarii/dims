// Copyright (c) 2014 Dims dev-team
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

namespace client
{

class CNodeConnectionManager
{
public:
	~CNodeConnectionManager();

	static CNodeConnectionManager* getInstance();

	void connectToNetwork();

	void periodicActionLoop();

	void executeAction();

private:
	CNodeConnectionManager();
private:
	static CNodeConnectionManager * ms_instance;

	common::CActionHandler< NodeResponses > * m_actionHandler;
};


}

#endif // NODE_CONNECTION_MANAGER_H
