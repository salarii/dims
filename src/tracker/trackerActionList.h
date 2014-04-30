// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef TRACKER_ACTION_LIST_H
#define TRACKER_ACTION_LIST_H

namespace tracker
{
class CGetBalanceAction;

typedef boost::mpl::list< CGetBalanceAction > TrackerActionList;

}

#endif // TRACKER_ACTION_LIST_H
