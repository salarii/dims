// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "configureMonitorActionHandler.h"
#include "common/actionHandler.h"
namespace monitor
{
template<>
unsigned int const common::CActionHandler< MonitorResponses >::m_sleepTime = 1000;
template<>
common::CActionHandler< MonitorResponses > * common::CActionHandler< MonitorResponses >::ms_instance = NULL;

}
