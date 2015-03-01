// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONFIGURE_NODE_ACTION_HADLER_H
#define CONFIGURE_NODE_ACTION_HADLER_H

#include "common/requestResponse.h"

#include "clientResponses.h"

namespace common
{
struct CAvailableCoins;

class CMediumException;

struct CClientNetworkInfoResult;

struct CTrackerSpecificStats;

template < class _Stats >
struct CNodeSpecific;

struct CTransactionAck;

struct CMonitorData;

struct CNoMedium;
}

namespace client
{

// list all desired types
typedef boost::mpl::list< common::CNoMedium, common::CMediumException, common::CTransactionStatus, common::CTransactionAck, common::CAccountBalance, common::CTrackerStats, common::CMonitorInfo, common::CPending, common::CAvailableCoins, CDnsInfo, common::CNodeSpecific< common::CClientNetworkInfoResult >, common::CNodeSpecific< common::CTrackerSpecificStats >, common::CNodeSpecific< common::CMonitorData > > NodeResponseList;

typedef boost::make_variant_over< NodeResponseList >::type NodeResponses;
}

#endif // CONFIGURE_NODE_ACTION_HADLER_H
