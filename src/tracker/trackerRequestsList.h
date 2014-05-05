#ifndef TRACKER_REQUESTS_LIST_H
#define TRACKER_REQUESTS_LIST_H

#include "getBalanceRequest.h"

namespace tracker
{
class CValidateTransactionsRequest;

typedef boost::mpl::list< CGetBalanceRequest, CValidateTransactionsRequest > TrackerRequestsList;

}

#endif // TRACKER_REQUESTS_LIST_H
