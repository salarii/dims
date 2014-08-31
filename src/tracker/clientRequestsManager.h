// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TASK_QUEUE_H
#define TASK_QUEUE_H

#include "common/nodeMessages.h"
#include "common/commonResponses.h"

namespace tracker
{

typedef boost::variant< common::CDummy, common::CAvailableCoins, common::CClientNetworkInfoResult, common::CTransactionAck, common::CTransactionStatusResponse, common::CTrackerSpecificStats > ClientResponse;

class CClientRequestsManager
{
public:
	uint256 addRequest( common::NodeRequest const & _nodeRequest );

	void addRequest( common::NodeRequest const & _nodeRequest, uint256 const & _hash );

	ClientResponse getResponse( uint256 const & _token );

	void processRequestLoop();

	void setClientResponse( uint256 const & _hash, ClientResponse const & _clientResponse );

	static CClientRequestsManager* getInstance();
private:
	typedef std::map< uint256, common::NodeRequest > InfoRequestRecord;
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
