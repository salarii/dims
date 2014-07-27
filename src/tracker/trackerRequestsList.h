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
}

namespace tracker
{

class CValidateTransactionsRequest;
class CConnectToTrackerRequest;
class CAskForTransactionsRequest;
class CSetBloomFilterRequest;
class CGetSynchronizationInfoRequest;
class CGetNextBlockRequest;
class CSetNextBlockRequest;
class CTransactionsKnownRequest;
class CTransactionsAckRequest;
class CTransactionsDoublespendRequest;
class CTransactionsNotOkRequest;

typedef boost::mpl::list< CGetBalanceRequest, CValidateTransactionsRequest, CConnectToTrackerRequest, common::CIdentifyRequest<TrackerResponses>, common::CIdentifyResponse<TrackerResponses>, common::CContinueReqest<TrackerResponses>, CAskForTransactionsRequest, CSetBloomFilterRequest, CGetSynchronizationInfoRequest, CGetNextBlockRequest, CSetNextBlockRequest, CTransactionsKnownRequest, CTransactionsAckRequest, CTransactionsDoublespendRequest, CTransactionsNotOkRequest, common::CNetworkRoleRequest< TrackerResponses >, common::CKnownNetworkInfoRequest< TrackerResponses > > TrackerRequestsList;

}

#endif // TRACKER_REQUESTS_LIST_H
