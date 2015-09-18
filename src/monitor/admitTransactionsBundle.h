// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ADMIT_TRANSACTIONS_BUNDLE_H
#define ADMIT_TRANSACTIONS_BUNDLE_H

#include "core.h"

#include <boost/statechart/state_machine.hpp>

#include "common/action.h"

namespace monitor
{

struct CWaitForBundle;

// temporary solution

// I base on fact  that  various  nodes  handling the  same transaction  bundle  should  use  the  sema  action  number
class CAdmitTransactionBundle : public common::CAction< common::CMonitorTypes >, public  boost::statechart::state_machine< CAdmitTransactionBundle, CWaitForBundle >
{
public:
	CAdmitTransactionBundle( uint256 const & _actionKey );

	virtual void accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor );

	~CAdmitTransactionBundle(){};
private:
};



}

#endif // ADMIT_TRANSACTIONS_BUNDLE_H
