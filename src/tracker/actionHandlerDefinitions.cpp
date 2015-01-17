// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "configureTrackerActionHandler.h"
#include "common/actionHandler.h"
namespace tracker
{
template<>
unsigned int const common::CActionHandler< TrackerResponses >::m_sleepTime = 100;
template<>
common::CActionHandler< TrackerResponses > * common::CActionHandler< TrackerResponses >::ms_instance = NULL;

}
