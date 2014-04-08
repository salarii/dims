// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_CONNECTION_MANAGER_H
#define NODE_CONNECTION_MANAGER_H

#include <set>
#include <QString>

#include "connectionProvider.h"

namespace node
{

class CActionHandler;

class CNodeConnectionManager
{
public:
	~CNodeConnectionManager();

	static CNodeConnectionManager* getInstance();

	void connectToNetwork();

	void executeAction();

private:
	CNodeConnectionManager();
private:
	static CNodeConnectionManager * ms_instance;

	CActionHandler * m_actionHandler;
};


}

#endif // NODE_CONNECTION_MANAGER_H