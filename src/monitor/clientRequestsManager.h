// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "common/nodeMessages.h"
#include "common/responses.h"

namespace monitor
{

typedef boost::variant< common::CAvailableCoinsData, common::CClientNetworkInfoResult, common::CTransactionAck, common::CTransactionStatusResponse, common::CTrackerSpecificStats, common::CMonitorData > ClientResponse;

class CClientRequestsManager
{
public:
	uint256 addRequest( common::NodeRequests const & _nodeRequest );

	void addRequest( common::NodeRequests const & _nodeRequest, uint256 const & _hash );

	bool getResponse( uint256 const & _token, ClientResponse & _clientResponse );

	void processRequestLoop();

	void setClientResponse( uint256 const & _hash, ClientResponse const & _clientResponse );

	static CClientRequestsManager* getInstance();
private:
	typedef std::map< uint256, common::NodeRequests > InfoRequestRecord;
	typedef std::map< uint256, ClientResponse > InfoResponseRecord;
private:
	CClientRequestsManager();
private:
	static CClientRequestsManager * ms_instance;

	mutable boost::mutex m_lock;
	mutable boost::mutex m_requestLock;

	InfoRequestRecord m_getInfoRequest;
	InfoResponseRecord m_infoResponseRecord;
	static uint256 ms_currentToken;

};


}

#endif
