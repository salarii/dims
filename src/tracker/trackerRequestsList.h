#ifndef TRACKER_REQUESTS_LIST_H
#define TRACKER_REQUESTS_LIST_H

#include "getBalanceRequest.h"

namespace tracker
{
class CValidateTransactionsRequest; class CConnectToTrackerRequest;class CIdentifyRequest;

typedef boost::mpl::list< CGetBalanceRequest, CValidateTransactionsRequest, CConnectToTrackerRequest, CIdentifyRequest > TrackerRequestsList;

}

#endif // TRACKER_REQUESTS_LIST_H
