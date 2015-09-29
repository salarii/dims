// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UPDATE_DATA_ACTION_H
#define UPDATE_DATA_ACTION_H

#include "common/action.h"
#include "common/filters.h"

#include <boost/statechart/state_machine.hpp>

#include "protocol.h"

namespace monitor
{

struct CAskForUpdate;

// rework  this  sooner  or later

class CUpdateDataAction : public common::CAction, public  boost::statechart::state_machine< CUpdateDataAction, CAskForUpdate >
{
public:
	CUpdateDataAction( bool _autoDelete );

	virtual void accept( common::CSetResponseVisitor & _visitor );

	void reset();

	~CUpdateDataAction(){};
private:
};


}

#endif // UPDATE_DATA_ACTION_H
