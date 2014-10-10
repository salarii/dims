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

template < class RequestType >
class CKnownNetworkInfoRequest;

template < class RequestType >
class CAckRequest;

template < class RequestType >
class CNetworkRoleRequest;

}

namespace monitor
{

class CConnectToNodeRequest;

class CConnectCondition;

typedef boost::mpl::list< common::CIdentifyRequest< MonitorResponses >, common::CContinueReqest< MonitorResponses >, common::CIdentifyResponse< MonitorResponses >, common::CKnownNetworkInfoRequest< MonitorResponses >, common::CAckRequest< MonitorResponses >, common::CNetworkRoleRequest< MonitorResponses >, CConnectToNodeRequest, CConnectCondition > MonitorRequestsList;

}

#endif // MONITOR_REQUESTS_LIST_H
