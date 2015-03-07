#ifndef SEED_REQUESTS_LIST_H
#define SEED_REQUESTS_LIST_H

#include "configureSeedActionHandler.h"

namespace common
{

template < class RequestType >
class CContinueReqest;

template < class RequestType >
class CSendIdentifyDataRequest;

template < class RequestType >
class CConnectToNodeRequest;

template < class RequestType >
class CNetworkRoleRequest;

template < class RequestType >
class CAckRequest;

template < class RequestType >
class CKnownNetworkInfoRequest;
}

#endif // SEED_REQUESTS_LIST_H
