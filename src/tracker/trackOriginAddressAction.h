// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRACK_ORIGIN_ADDRESS_ACTION_H
#define TRACK_ORIGIN_ADDRESS_ACTION_H

#include "common/action.h"
#include "configureTrackerActionHandler.h"
#include <boost/statechart/state_machine.hpp>

namespace tracker
{
struct CUninitiated;

class CTrackOriginAddressAction : public common::CAction< TrackerResponses >, public  boost::statechart::state_machine< CTrackOriginAddressAction, CUninitiated >
{
public:
	CTrackOriginAddressAction();

	virtual common::CRequest< TrackerResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< TrackerResponses > & _visitor );

	void setRequest( common::CRequest< TrackerResponses >* _request );
private:
	common::CRequest< TrackerResponses >* m_request;
};


}

#endif // TRACK_ORIGIN_ADDRESS_ACTION_H
