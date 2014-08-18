#ifndef CONFIGURE_NODE_ACTION_HADLER_H
#define CONFIGURE_NODE_ACTION_HADLER_H

#include "common/requestResponse.h"

#include "clientResponses.h"

namespace common
{
struct CAvailableCoins;

class CMediumException;

struct CClientNetworkInfoResult;

struct CContinueResult;

struct CTrackerSpecificStats;

template < class _Stats >
struct CNodeSpecific;

}

namespace client
{

// list all desired types
typedef boost::mpl::list< common::CMediumException, common::CTransactionStatus, common::CAccountBalance, common::CTrackerStats, common::CMonitorInfo, common::CPending, common::CAvailableCoins, CDnsInfo, common::CNodeSpecific< common::CClientNetworkInfoResult >, common::CContinueResult, common::CNodeSpecific< common::CTrackerSpecificStats > > NodeResponseList;

typedef boost::make_variant_over< NodeResponseList >::type NodeResponses;
}

#endif // CONFIGURE_NODE_ACTION_HADLER_H
