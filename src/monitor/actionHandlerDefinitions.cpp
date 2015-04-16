// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "configureMonitorActionHandler.h"
#include "common/actionHandler.h"
#include "common/periodicActionExecutor.h"
#include "common/timeMedium.h"
#include "common/scheduledActionManager.h"
#include "common/scheduleAbleAction.h"

namespace monitor
{
template<>
unsigned int const common::CActionHandler< common::CMonitorTypes >::m_sleepTime = 1000;
template<>
common::CActionHandler< common::CMonitorTypes > * common::CActionHandler< common::CMonitorTypes >::ms_instance = NULL;

template<>
common::CPeriodicActionExecutor< common::CMonitorTypes > * common::CPeriodicActionExecutor< common::CMonitorTypes >::ms_instance = NULL;

template<>
unsigned int const common::CPeriodicActionExecutor< common::CMonitorTypes >::m_sleepTime = 100;

template<>
common::CTimeMedium< common::CMonitorBaseMedium > * common::CTimeMedium< common::CMonitorBaseMedium >::ms_instance = NULL;

}
