// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "configureTrackerActionHandler.h"
#include "common/actionHandler.h"
#include "common/timeMedium.h"
#include "common/scheduledActionManager.h"

namespace tracker
{
template<>
unsigned int const common::CActionHandler< common::CTrackerTypes >::m_sleepTime = 100;

template<>
common::CActionHandler< common::CTrackerTypes > * common::CActionHandler< common::CTrackerTypes >::ms_instance = NULL;

template<>
common::CTimeMedium< common::CTrackerBaseMedium > * common::CTimeMedium< common::CTrackerBaseMedium >::ms_instance = NULL;

template<>
common::CScheduledActionManager< common::CTrackerTypes > * common::CScheduledActionManager< common::CTrackerTypes >::ms_instance = NULL;

}
