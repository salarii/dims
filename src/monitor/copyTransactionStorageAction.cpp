// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/commonRequests.h"
#include "common/commonEvents.h"
#include "common/mediumKinds.h"
#include "common/originAddressScanner.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "monitor/monitorController.h"
#include "monitor/filters.h"
#include "monitor/monitorNodeMedium.h"
#include "monitor/copyTransactionStorageAction.h"

namespace monitor
{
// send  info  about  copy if  it  exist  and  what  size  it  is
// request  for  downloading
// provide fragments
//  provide  missing transactions
// flag operating and ready


CCopyTransactionStorageAction::CCopyTransactionStorageAction()
{}

void
CCopyTransactionStorageAction::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
