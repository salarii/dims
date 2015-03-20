#ifndef TRACKER_REQUESTS_LIST_H
#define TRACKER_REQUESTS_LIST_H

namespace common
{
template < class _Types >
class CSendIdentifyDataRequest;

template < class _Types >
class CNetworkRoleRequest;

template < class _Types >
class CKnownNetworkInfoRequest;

template < class _Types >
class CAckRequest;

template < class _Types >
class CEndRequest;

template < class _Types >
class CResultRequest;

template < class _Types >
class CTimeEventRequest;

template < class _Types >
class CPingRequest;

template < class _Types >
class CPongRequest;
}

namespace tracker
{

class CValidateTransactionsRequest;
class CConnectToTrackerRequest;
class CAskForTransactionsRequest;
class CSetBloomFilterRequest;
class CGetSynchronizationInfoRequest;
class CGetNextBlockRequest;
class CTransactionsStatusRequest;
class CTransactionsPropagationRequest;
class CPassMessageRequest;
class CDeliverInfoRequest;
class CGetBalanceRequest;

template < class Block >
class CSetNextBlockRequest;

struct CSegmentHeader;
struct CDiskBlock;

}

#endif // TRACKER_REQUESTS_LIST_H
