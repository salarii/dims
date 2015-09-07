// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "commonResponses.h"

namespace common
{

CMainRequestType::Enum const CAvailableCoinsEvent::m_requestType = CMainRequestType::BalanceInfoReq;

CAvailableCoinsEvent::CAvailableCoinsEvent( std::map< uint256, CCoins > const & _availableCoins, uint256 const & _hash )
	: m_hash(_hash)
	, m_availableCoins( _availableCoins )
{
}

CAvailableCoinsEvent::CAvailableCoinsEvent()
{

}

}
