// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODE_REQUEST_LIST_H
#define NODE_REQUEST_LIST_H

#include "configureClientActionHadler.h"

namespace client
{
struct CBalanceRequest;
struct CInfoRequestContinue;
struct CTransactionStatusRequest;
struct CTransactionSendRequest;
struct CTrackersInfoRequest;
struct CMonitorInfoRequest;
struct CDnsInfoRequest;
struct CRecognizeNetworkRequest;
struct CErrorForAppPaymentProcessing;
struct CProofTransactionAndStatusRequest;

typedef boost::mpl::list< CBalanceRequest, CInfoRequestContinue, CTransactionStatusRequest, CTransactionSendRequest, CTrackersInfoRequest, CMonitorInfoRequest, CDnsInfoRequest, CRecognizeNetworkRequest, CErrorForAppPaymentProcessing, CProofTransactionAndStatusRequest > NodeRequestsList;

}

#endif // NODE_REQUEST_LIST_H
