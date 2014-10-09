// Copyright (c) 2014 Dims dev-team
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

typedef boost::mpl::list< common::CMediumException, CDummyResponse, common::CIdentificationResult, common::CContinueResult, common::CAckPromptResult, common::CConnectedNode, common::CAckResult, common::CMessageResult > MonitorResponseList;

typedef boost::make_variant_over< MonitorResponseList >::type MonitorResponses;

}


#endif // CONFIGURE_MONITOR_ACTION_HANDLER_H
