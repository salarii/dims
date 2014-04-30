#include "transactionRecordManager.h"

#include "txdb.h"

#include "txmempool.h"

#include "main.h"
#include "addressToCoins.h"


namespace tracker
{

// -dbcache default (MiB)
static const int64_t nDefaultDbCache = 100;
// max. -dbcache in (MiB)
static const int64_t nMaxDbCache = sizeof(void*) > 4 ? 4096 : 1024;
// min. -dbcache in (MiB)
static const int64_t nMinDbCache = 4;


CTransactionRecordManager * CTransactionRecordManager::ms_instance = NULL;

CTransactionRecordManager*
CTransactionRecordManager::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CTransactionRecordManager();
	};
	return ms_instance;
}

CTransactionRecordManager::CTransactionRecordManager()
:scriptcheckqueue(32)
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

//add  passing  cache  size ??
	m_addressToCoinsViewCache = CAddressToCoinsViewCache::getInstance(  );
}

CTransactionRecordManager::~CTransactionRecordManager()
{
	delete m_coinsViewCache;
	delete m_memPool;
}

void
CTransactionRecordManager::addCoinbaseTransaction( CTransaction const & _tx, uint160 const & _keyId  )
{
	CCoins coins(_tx, 0);
	boost::lock_guard<boost::mutex> lock( m_coinsViewLock );

	m_coinsViewCache->SetCoins(_tx.GetHash() , coins);
	m_addressToCoinsViewCache->setCoins( _keyId, _tx.GetHash() );

	bool pfMissingInputs;
	CValidationState state;
	AcceptToMemoryPool(*m_memPool, state, _tx, false, &pfMissingInputs, false);

}

bool
CTransactionRecordManager::checkIfCoinsAvailable( CTransaction const & _tx ) const
{
	m_coinsViewCache->HaveInputs(_tx);
}

bool
CTransactionRecordManager::validateTransactionBundle( std::vector< CTransaction > const & _transaction )
{
	CValidationState state;
	unsigned int flags = SCRIPT_VERIFY_NOCACHE | SCRIPT_VERIFY_NONE;

	CCheckQueueControl<CScriptCheck> control( &scriptcheckqueue);

	BOOST_FOREACH( CTransaction const & transaction, _transaction )
	{
			std::vector<CScriptCheck> vChecks;
		if ( !CheckInputs(transaction, state, *m_coinsViewCache, true, flags, &vChecks ) )
			return false;

		control.Add(vChecks);
	}
	return control.Wait();
}

bool
CTransactionRecordManager::getCoins( std::vector< uint256 > const & _transaction,  std::vector< CCoins > & _coins ) const
{
	CCoins coins;
	BOOST_FOREACH( uint256 const & hash, _transaction )
	{
		if ( m_coinsViewCache->GetCoins( hash, coins ) )
			return false;
		_coins.push_back( coins );
	}
	return true;
}

void
CTransactionRecordManager::loop( std::vector< CTransaction > const & _transaction )
{
	while(1)
	{

		//update  pool  and view  with valid transaction
		// send  those to storage

		boost::this_thread::interruption_point();
	}

}

void
CTransactionRecordManager::synchronize()
{
}

void
CTransactionRecordManager::askForTokens()
{
}

}
