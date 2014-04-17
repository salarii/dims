// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "nodeMessages.h"

namespace tracker
{

class CClientRequestsManager
{
public:
	uint256 addRequest( NodeRequest const & _nodeRequest );

	void processRequestLoop();
private:
	typedef std::map< uint256, NodeRequest > InfoRequestElement;
private:
    mutable boost::mutex m_lock;
	InfoRequestElement m_getInfoRequest;

	static uint256 ms_currentToken;
};


}

#endif
