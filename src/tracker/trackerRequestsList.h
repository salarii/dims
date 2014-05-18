#ifndef TRACKER_REQUESTS_LIST_H
#define TRACKER_REQUESTS_LIST_H

#include "getBalanceRequest.h"

namespace tracker
{
class CValidateTransactionsRequest; class CConnectToTrackerRequest;

typedef boost::mpl::list< CGetBalanceRequest, CValidateTransactionsRequest, CConnectToTrackerRequest > TrackerRequestsList;

}

#endif // TRACKER_REQUESTS_LIST_H
