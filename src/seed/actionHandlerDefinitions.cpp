// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "configureSeedActionHandler.h"
#include "common/actionHandler.h"
#include "common/timeMedium.h"

namespace seed
{

template<>
unsigned int const common::CActionHandler< SeedResponses >::m_sleepTime = 100;
template<>
common::CActionHandler< SeedResponses > * common::CActionHandler< SeedResponses >::ms_instance = NULL;
template<>
common::CTimeMedium< SeedResponses > * common::CTimeMedium< SeedResponses >::ms_instance = NULL;

}
