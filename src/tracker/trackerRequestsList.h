#ifndef TRACKER_REQUESTS_LIST_H
#define TRACKER_REQUESTS_LIST_H

namespace common
{
template < class ResponsesType >
class CIdentifyResponse;

template < class ResponsesType >
class CNetworkRoleRequest;

template < class ResponsesType >
class CKnownNetworkInfoRequest;

template < class ResponsesType >
class CAckRequest;

template < class ResponsesType >
class CEndRequest;

template < class ResponsesType >
class CResultRequest;

template < class ResponsesType >
class CTimeEventRequest;
}

namespace tracker
{

class CValidateTransactionsRequest;
class CConnectToTrackerRequest;
class CAskForTransactionsRequest;
class CSetBloomFilterRequest;
class CGetSynchronizationInfoRequest;
class CGetNextBlockRequest;
class CTransactionsStatusRequest;
class CTransactionsPropagationRequest;
class CPassMessageRequest;
class CDeliverInfoRequest;
class CGetBalanceRequest;

template < class Block >
class CSetNextBlockRequest;

struct CSegmentHeader;
struct CDiskBlock;

}

#endif // TRACKER_REQUESTS_LIST_H
