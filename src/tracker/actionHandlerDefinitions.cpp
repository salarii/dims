// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "configureTrackerActionHandler.h"
#include "common/actionHandler.h"
#include "common/timeMedium.h"

namespace tracker
{
template<>
unsigned int const common::CActionHandler< CTrackerTypes >::m_sleepTime = 100;

template<>
common::CActionHandler< CTrackerTypes > * common::CActionHandler< CTrackerTypes >::ms_instance = NULL;

template<>
common::CTimeMedium< TrackerResponses > * common::CTimeMedium< TrackerResponses >::ms_instance = NULL;
}
