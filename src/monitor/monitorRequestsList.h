// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef MONITOR_REQUESTS_LIST_H
#define MONITOR_REQUESTS_LIST_H

#include "configureMonitorActionHandler.h"

namespace common
{

template < class RequestType >
class CContinueReqest;

template < class RequestType >
class CSendIdentifyDataRequest;

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

class CRegistrationTerms;

class CInfoRequest;
}

#endif // MONITOR_REQUESTS_LIST_H
