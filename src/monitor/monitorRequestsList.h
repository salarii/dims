#ifndef MONITOR_REQUESTS_LIST_H
#define MONITOR_REQUESTS_LIST_H

#include "configureMonitorActionHandler.h"

namespace common
{
template < class RequestType >
class CIdentifyRequest;

template < class RequestType >
class CContinueReqest;

template < class RequestType >
class CIdentifyResponse;
}

namespace monitor
{

typedef boost::mpl::list< common::CIdentifyRequest< MonitorResponses >, common::CContinueReqest< MonitorResponses >, common::CIdentifyResponse< MonitorResponses > > MonitorRequestsList;

}

#endif // MONITOR_REQUESTS_LIST_H
