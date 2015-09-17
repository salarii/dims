// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONFIGURE_CLIENT_ACTION_HADLER_H
#define CONFIGURE_CLIENT_ACTION_HADLER_H

#include "common/commonResponses.h"

#include "common/clientProtocol.h"

#include "clientResponses.h"

namespace client
{

// list all desired types
typedef boost::mpl::list<
common::CNoMedium
, common::CTimeEvent
, common::CMediumException
, common::CClientMessageResponse
, CDnsInfo > ClientResponseList;

typedef boost::make_variant_over< ClientResponseList >::type ClientResponses;
}

#endif // CONFIGURE_CLIENT_ACTION_HADLER_H
