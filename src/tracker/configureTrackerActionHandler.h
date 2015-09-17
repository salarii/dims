// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONFIGURE_TRACKER_ACTION_HANDLER_H
#define CONFIGURE_TRACKER_ACTION_HANDLER_H

#include <boost/mpl/list.hpp>
#include <boost/variant/variant.hpp>

#include "common/commonResponses.h"
#include "common/commonEvents.h"

#include "main.h"

namespace tracker
{
typedef common::DimsResponseList TrackerResponseList;
typedef common::DimsResponses TrackerResponses;
}

#endif // CONFIGURE_TRACKER_ACTION_HANDLER_H
