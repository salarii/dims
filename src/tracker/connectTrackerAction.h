// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECT_TRACKER_ACTION_H
#define CONNECT_TRACKER_ACTION_H

#include "common/action.h"
#include "configureTrackerActionHandler.h"

namespace tracker
{

class CConnectTrackerAction : public common::CAction< TrackerResponses >
{
public:
	CConnectTrackerAction( std::string const & _trackerAddress );

	virtual common::CRequest< TrackerResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< TrackerResponses > & _visitor );
private:
	std::string const & m_trackerAddress;
};


}

#endif // CONNECT_TRACKER_ACTION_H
