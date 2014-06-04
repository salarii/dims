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
}

namespace tracker
{
class CValidateTransactionsRequest;
class CConnectToTrackerRequest;

typedef boost::mpl::list< CGetBalanceRequest, CValidateTransactionsRequest, CConnectToTrackerRequest, common::CIdentifyRequest<TrackerResponses>, common::CIdentifyResponse<TrackerResponses>, common::CContinueReqest<TrackerResponses> > TrackerRequestsList;

}

#endif // TRACKER_REQUESTS_LIST_H
