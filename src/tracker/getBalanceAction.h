// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef GET_BALANCE_ACTION_H
#define GET_BALANCE_ACTION_H

#include "configureTrackerActionHandler.h"
#include "common/action.h"

namespace tracker
{

class CGetBalanceAction : public common::CAction< TrackerResponses >
{
public:
	virtual void accept( common::CSetResponseVisitorBase< TrackerResponses > & _visitor )
	{

	}

	virtual common::CRequest< TrackerResponses >* execute()
	{ return 0; }

	virtual void reset(){}
};

}

#endif // GET_BALANCE_ACTION_H
