#include "transactionRecordManager.h"

#include "txdb.h"

#include "txmempool.h"

#include "main.h"

namespace self
{

// -dbcache default (MiB)
static const int64_t nDefaultDbCache = 100;
// max. -dbcache in (MiB)
static const int64_t nMaxDbCache = sizeof(void*) > 4 ? 4096 : 1024;
// min. -dbcache in (MiB)
static const int64_t nMinDbCache = 4;


CTransactionRecordManager::CTransactionRecordManager()
{
	// cache size calculations
	size_t nTotalCache = (GetArg("-dbcache", nDefaultDbCache) << 20);
	if (nTotalCache < (nMinDbCache << 20))
	    nTotalCache = (nMinDbCache << 20); // total cache cannot be less than nMinDbCache
	else if (nTotalCache > (nMaxDbCache << 20))
	    nTotalCache = (nMaxDbCache << 20); // total cache cannot be greater than nMaxDbCache
	size_t nBlockTreeDBCache = nTotalCache / 8;
	if (nBlockTreeDBCache > (1 << 21) && !GetBoolArg("-txindex", false))
	    nBlockTreeDBCache = (1 << 21); // block tree db cache shouldn't be larger than 2 MiB
	nTotalCache -= nBlockTreeDBCache;
	size_t nCoinDBCache = nTotalCache / 2; // use half of the remaining cache for coindb cache
	nTotalCache -= nCoinDBCache;
	nCoinCacheSize = nTotalCache / 300; // coins in memory require around 300 bytes

	m_coinsViewCache = new CCoinsViewCache(*(new CCoinsViewDB(nCoinDBCache, false, fReindex)));
	m_memPool = new CTxMemPool;
}

CTransactionRecordManager::~CTransactionRecordManager()
{
	delete m_coinsViewCache;
	delete m_memPool;
}

void
CTransactionRecordManager::addCoinbaseTransaction( CTransaction const & _tx )
{
	CCoins coins(_tx, 0);
	m_coinsViewCache->SetCoins(_tx.GetHash() , coins);

	bool* pfMissingInputs;
	CValidationState state;
	AcceptToMemoryPool(*m_memPool, state, _tx, false, pfMissingInputs, false);

}

bool
CTransactionRecordManager::checkIfCoinsAvailable( CTransaction const & _tx ) const
{
	m_coinsViewCache->HaveInputs(_tx);
}


void CTransactionRecordManager::validateTransaction( CTransaction const & _tx )
{

}

void
CTransactionRecordManager::handleTransactionBundle( std::vector< CTransaction > const & _transaction )
{
}

void
CTransactionRecordManager::loop( std::vector< CTransaction > const & _transaction )
{
	while(1)
	{
		// bundle  from  validation  queue pass to  validator

		// check  for  job  done by  validator  periodically

		//update  pool  and view  with  transaction  which  are  considered as valid




	    boost::this_thread::interruption_point();
	}

}

void
// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

CTransactionRecordManager::synchronize()
{
}

void
CTransactionRecordManager::askForTokens()
{
}

}
