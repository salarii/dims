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
unsigned int const common::CActionHandler< ClientResponses >::m_sleepTime = 100;
template<>
common::CActionHandler< ClientResponses > * common::CActionHandler< ClientResponses >::ms_instance = NULL;

template<>
common::CPeriodicActionExecutor< ClientResponses > * common::CPeriodicActionExecutor< ClientResponses >::ms_instance = NULL;

template<>
unsigned int const common::CPeriodicActionExecutor< ClientResponses >::m_sleepTime = 100;

template<>
common::CTimeMedium< ClientResponses > * common::CTimeMedium< ClientResponses >::ms_instance = NULL;

}
