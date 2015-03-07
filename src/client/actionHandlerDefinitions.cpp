// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "configureClientActionHadler.h"
#include "common/actionHandler.h"
#include "common/periodicActionExecutor.h"
#include "common/timeMedium.h"

namespace client
{
template<>
unsigned int const common::CActionHandler< common::CClientTypes >::m_sleepTime = 100;
template<>
common::CActionHandler< common::CClientTypes > * common::CActionHandler< common::CClientTypes >::ms_instance = NULL;

template<>
common::CPeriodicActionExecutor< common::CClientTypes > * common::CPeriodicActionExecutor< common::CClientTypes >::ms_instance = NULL;

template<>
unsigned int const common::CPeriodicActionExecutor< common::CClientTypes >::m_sleepTime = 100;

template<>
common::CTimeMedium< common::CClientBaseMedium > * common::CTimeMedium< common::CClientBaseMedium >::ms_instance = NULL;

}
