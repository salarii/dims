// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONFIGURE_TRACKER_ACTION_HANDLER_H
#define CONFIGURE_TRACKER_ACTION_HANDLER_H

#include <boost/mpl/list.hpp>
#include <boost/variant/variant.hpp>

#include "common/trackerResponses.h"

namespace tracker
{
struct CDummyResponse{};

// list all desired types
typedef boost::mpl::list< CDummyResponse, common::CAvailableCoins > TrackerResponseList;

typedef boost::make_variant_over< TrackerResponseList >::type TrackerResponses;

}

#endif // CONFIGURE_TRACKER_ACTION_HANDLER_H
