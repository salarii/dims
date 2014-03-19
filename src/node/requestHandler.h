#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

namespace node
{

struct CTransactionStatus
{
	TransactionsStatus::Enum m_status;
	uint256 m_token;
};

struct CAccountBalance
{

};

struct CRequest
{
	void serialize( CBufferAsStream & _bufferStream ) = 0;
};

class CRequestHandler
{
public:
	RequestRespond getRespond( CRequest* _request ) const;

	bool isProcessed( CRequest* _request ) const;
private:
	std::vector<CRequest*> m_newRequest
	std::map<CRequest*,uint256> m_pendingRequest;
	std::map<CRequest*,RequestRespond> m_processedRequests;
};


m_processedRequests.find( reqAction.first )

/*

CClientMessageType::Enum
Transaction
, TrackerInfoReq
, MonitorInfoReq
, TransactionInfoReq

*/

}

#endif