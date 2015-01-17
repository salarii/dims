// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "configureNodeActionHadler.h"
#include "common/actionHandler.h"
#include "common/periodicActionExecutor.h"

namespace client
{
template<>
unsigned int const common::CActionHandler< NodeResponses >::m_sleepTime = 100;
template<>
common::CActionHandler< NodeResponses > * common::CActionHandler< NodeResponses >::ms_instance = NULL;

template<>
common::CPeriodicActionExecutor< NodeResponses > * common::CPeriodicActionExecutor< NodeResponses >::ms_instance = NULL;

template<>
unsigned int const common::CPeriodicActionExecutor< NodeResponses >::m_sleepTime = 100;
}
