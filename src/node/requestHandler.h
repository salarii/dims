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
	bool isProcessed( CRequest* _request );
private:
	std::vector<CRequest*> m_newRequest
	std::map<CRequest*,uint256> m_pendingRequest;
	//std::map<CRequest*,uint256> m_pendingRequest;
};

/*

CClientMessageType::Enum
Transaction
, TrackerInfoReq
, MonitorInfoReq
, TransactionInfoReq

*/

}

#endif