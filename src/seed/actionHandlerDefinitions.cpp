// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "configureSeedActionHandler.h"
#include "common/actionHandler.h"
#include "common/timeMedium.h"

namespace seed
{

template<>
unsigned int const common::CActionHandler< common::CSeedTypes >::m_sleepTime = 100;
template<>
common::CActionHandler< common::CSeedTypes > * common::CActionHandler< common::CSeedTypes >::ms_instance = NULL;
template<>
common::CTimeMedium< common::CSeedBaseMedium > * common::CTimeMedium< common::CSeedBaseMedium >::ms_instance = NULL;

}
