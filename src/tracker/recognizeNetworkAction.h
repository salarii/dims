// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef RECOGNIZE_NETWORK_ACTION_H
#define RECOGNIZE_NETWORK_ACTION_H

#include "common/action.h"
#include "common/filters.h"

#include <boost/statechart/state_machine.hpp>

namespace tracker
{

struct CGetDnsInfo;

class CRecognizeNetworkAction : public common::CAction, public  boost::statechart::state_machine< CRecognizeNetworkAction, CGetDnsInfo >
{
public:
	CRecognizeNetworkAction();

	virtual void accept( common::CSetResponseVisitor & _visitor );

private:
};


}

#endif // RECOGNIZE_NETWORK_ACTION_H
