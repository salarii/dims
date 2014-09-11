#ifndef TRACKER_REQUESTS_LIST_H
#define TRACKER_REQUESTS_LIST_H

#include "getBalanceRequest.h"

namespace common
{
template < class RequestType >
class CIdentifyRequest;

template < class RequestType >
class CContinueReqest;

template < class RequestType >
class CIdentifyResponse;

template < class RequestType >
class CNetworkRoleRequest;

template < class RequestType >
class CKnownNetworkInfoRequest;

template < class RequestType >
class CAckRequest;

template < class RequestType >
class CEndRequest;

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

template < class Block >
class CSetNextBlockRequest;

struct CSegmentHeader;
struct CDiskBlock;

}

#endif // TRACKER_REQUESTS_LIST_H
