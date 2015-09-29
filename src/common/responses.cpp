// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/responses.h"

namespace common
{

CMainRequestType::Enum const CAvailableCoinsData::m_requestType = CMainRequestType::BalanceInfoReq;

CAvailableCoinsData::CAvailableCoinsData(
		std::map< uint256
		, CCoins > const & _availableCoins
		, std::map< uint256, std::vector< CKeyID > > const &_transactionInputs
		, uint256 const & _hash )
	: m_hash(_hash)
	, m_availableCoins( _availableCoins )
	, m_transactionInputs( _transactionInputs )
{
}

CAvailableCoinsData::CAvailableCoinsData()
{
}

}
