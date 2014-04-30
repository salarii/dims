// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "internalOperationsMedium.h"
#include "addressToCoins.h"
#include "transactionRecordManager.h"

#include <algorithm>

namespace tracker
{

CInternalOperationsMedium * CInternalOperationsMedium::ms_instance = NULL;

CInternalOperationsMedium*
CInternalOperationsMedium::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CInternalOperationsMedium();
	};
	return ms_instance;
}

void
CInternalOperationsMedium::add( CGetBalanceRequest const *_request )
{
	common::CAvailableCoins availableCoins;

	std::vector< uint256 > coinsHashes;
	CAddressToCoinsViewCache::getInstance()->getCoins( _request->getKey(), coinsHashes );

	std::vector< CCoins > coins;
	CTransactionRecordManager::getInstance()->getCoins( coinsHashes, coins );

	std::transform( coinsHashes.begin(), coinsHashes.end(), coins.begin(),
		   std::inserter(availableCoins.m_availableCoins2, availableCoins.m_availableCoins2.end() ), std::make_pair<uint256,CCoins> );
}

}
