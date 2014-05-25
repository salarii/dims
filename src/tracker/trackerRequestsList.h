#ifndef TRACKER_REQUESTS_LIST_H
#define TRACKER_REQUESTS_LIST_H

#include "getBalanceRequest.h"

namespace tracker
{
class CValidateTransactionsRequest; class CConnectToTrackerRequest;class CIdentifyRequest;class CIdentifyResponse; class CContinueReqest;

typedef boost::mpl::list< CGetBalanceRequest, CValidateTransactionsRequest, CConnectToTrackerRequest, CIdentifyRequest, CIdentifyResponse, CContinueReqest > TrackerRequestsList;

}

#endif // TRACKER_REQUESTS_LIST_H
