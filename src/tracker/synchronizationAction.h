// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SYNCHRONIZATION_ACTION_H
#define SYNCHRONIZATION_ACTION_H

#include "common/action.h"
#include "configureTrackerActionHandler.h"
#include <boost/statechart/state_machine.hpp>

namespace tracker
{

struct CUninitiated;

class CSynchronizationAction : public common::CAction< TrackerResponses >, public  boost::statechart::state_machine< CSynchronizationAction, CUninitiated >
{
public:
	CSynchronizationAction();

	virtual common::CRequest< TrackerResponses >* execute();

	virtual void accept( common::CSetResponseVisitor< TrackerResponses > & _visitor );

	void setRequest( common::CRequest< TrackerResponses >* _request );

	void clear();
private:
	common::CRequest< TrackerResponses >* m_request;

	uint256 m_currentHash;
};

}

#endif // SYNCHRONIZATION_ACTION_H
