// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_REQUEST_LIST_H
#define NODE_REQUEST_LIST_H

#include "configureNodeActionHadler.h"

namespace common
{
template < class RequestType >
class CContinueReqest;

}

namespace client
{
struct CBalanceRequest;
struct CInfoRequestContinueComplex;
struct CInfoRequestContinue;
struct CTransactionStatusRequest;
struct CTransactionSendRequest;
struct CTrackersInfoRequest;
struct CMonitorInfoRequest;
struct CDnsInfoRequest;
struct CRecognizeNetworkRequest;

typedef boost::mpl::list< CBalanceRequest, CInfoRequestContinueComplex, CInfoRequestContinue, CTransactionStatusRequest, CTransactionSendRequest, CTrackersInfoRequest, CMonitorInfoRequest, CDnsInfoRequest, common::CContinueReqest< NodeResponses >, CRecognizeNetworkRequest > NodeRequestsList;

}

#endif // NODE_REQUEST_LIST_H
