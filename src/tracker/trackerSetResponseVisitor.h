// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACKER_SET_RESPONSE_VISITOR_H
#define TRACKER_SET_RESPONSE_VISITOR_H

#include "common/setResponseVisitor.h"
#include "configureTrackerActionHandler.h"

namespace tracker
{

class CTrackerSetResponseVisitor : public common::CSetResponseVisitor< TrackerResponses >
{
public:
	virtual void visit( common::CAction< TrackerResponses > & _action ){};
};


}

#endif // TRACKER_SET_RESPONSE_VISITOR_H
