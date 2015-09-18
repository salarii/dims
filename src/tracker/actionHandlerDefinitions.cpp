// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/actionHandler.h"
#include "common/timeMedium.h"
#include "common/scheduledActionManager.h"
#include "common/scheduleAbleAction.h"

namespace common
{
unsigned int const
common::CActionHandler::m_sleepTime = 100;

common::CActionHandler * common::CActionHandler::ms_instance = NULL;

common::CTimeMedium * common::CTimeMedium::ms_instance = NULL;

common::CScheduledActionManager * common::CScheduledActionManager::ms_instance = NULL;

}
