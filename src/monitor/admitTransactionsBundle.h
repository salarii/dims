// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ADMIT_TRANSACTIONS_BUNDLE_H
#define ADMIT_TRANSACTIONS_BUNDLE_H

#include "common/action.h"
#include "common/types.h"

#include <boost/statechart/state_machine.hpp>

namespace monitor
{

struct CWaitForBundle;
// this is weird action most likely it should be ordinary singleton
class CAdmitTransactionBundle : public common::CAction< common::CMonitorTypes >, public  boost::statechart::state_machine< CAdmitTransactionBundle, CWaitForBundle >, public common::CCommunicationAction
{
public:
	static CAdmitTransactionBundle* getInstance();

	virtual void accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor );

	~CAdmitTransactionBundle(){};
private:
	CAdmitTransactionBundle();

	static CAdmitTransactionBundle * ms_instance;
};



}

#endif // ADMIT_TRANSACTIONS_BUNDLE_H
