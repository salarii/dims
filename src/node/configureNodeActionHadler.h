#ifndef CONFIGURE_NODE_ACTION_HADLER_H
#define CONFIGURE_NODE_ACTION_HADLER_H

#include "common/requestResponse.h"

namespace common
{
struct CAvailableCoins;

}

namespace node
{
// list all desired types
typedef boost::mpl::list< common::CTransactionStatus, common::CAccountBalance, common::CTrackerStats, common::CMonitorInfo, common::CPending, common::CSystemError, common::CAvailableCoins > NodeResponseList;

typedef boost::make_variant_over< NodeResponseList >::type NodeResponses;
}

#endif // CONFIGURE_NODE_ACTION_HADLER_H
