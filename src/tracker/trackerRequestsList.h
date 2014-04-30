#ifndef TRACKER_REQUESTS_LIST_H
#define TRACKER_REQUESTS_LIST_H

#include "getBalanceRequest.h"

namespace tracker
{

typedef boost::mpl::list< CGetBalanceRequest > TrackerRequestsList;

}

#endif // TRACKER_REQUESTS_LIST_H
