#ifndef NODE_REQUEST_LIST_H
#define NODE_REQUEST_LIST_H

namespace client
{
struct CBalanceRequest;
struct CInfoRequestContinue;
struct CTransactionStatusRequest;
struct CTransactionSendRequest;
struct CTrackersInfoRequest;
struct CMonitorInfoRequest;

typedef boost::mpl::list< CBalanceRequest, CInfoRequestContinue, CTransactionStatusRequest, CTransactionSendRequest, CTrackersInfoRequest, CMonitorInfoRequest > NodeRequestsList;

}

#endif // NODE_REQUEST_LIST_H
