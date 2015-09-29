// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VALIDATE_TRANSACTION_ACTION_EVENTS_H
#define VALIDATE_TRANSACTION_ACTION_EVENTS_H

#include <boost/statechart/event.hpp>
#include "main.h"

namespace tracker
{

struct CSwitchToSynchronizing : boost::statechart::event< CSwitchToSynchronizing >
{
};

struct CSwitchToSynchronized : boost::statechart::event< CSwitchToSynchronized >
{
};

}


#endif // VALIDATE_TRANSACTION_ACTION_EVENTS_H
