// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONFIGURE_MONITOR_ACTION_HANDLER_H
#define CONFIGURE_MONITOR_ACTION_HANDLER_H

#include <boost/mpl/list.hpp>
#include <boost/variant/variant.hpp>
#include "common/commonResponses.h"


namespace monitor
{

struct CDummyResponse{};

typedef boost::mpl::list< common::ScheduledResult,common::CPingPongResult, common::CMediumException, CDummyResponse, common::CIdentificationResult, common::CConnectedNode, common::CAckResult, common::CMessageResult, common::CNoMedium, common::CTimeEvent, common::CRequestedMerkles > MonitorResponseList;

typedef boost::make_variant_over< MonitorResponseList >::type MonitorResponses;

}


#endif // CONFIGURE_MONITOR_ACTION_HANDLER_H
