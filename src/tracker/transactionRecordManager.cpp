#include "transactionRecordManager.h"

#include "txdb.h"

#include "txmempool.h"

#include "main.h"
#include "addressToCoins.h"

#include "common/actionHandler.h"

#include "validateTransactionsAction.h"


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

	// this  is  needed  but does not work like it should, so I commented it out
	//AcceptToMemoryPool(*m_memPool, state, _tx, false, &pfMissingInputs, false);
	m_addressToCoinsViewCache->flush();
	m_coinsViewCache->Flush();
}

bool
CTransactionRecordManager::addValidatedTransactionBundle( std::vector< CTransaction > const & _transaction )
{

	boost::lock_guard<boost::mutex> lock( m_coinsViewLock );

	BOOST_FOREACH( CTransaction const & transaction, _transaction )
	{
		BOOST_FOREACH( CTxIn const & txIn, transaction.vin )
		{
			CCoins coins;
			if ( !m_coinsViewCache->GetCoins(txIn.prevout.hash, coins) )
				return false;

			coins.vout.at( txIn.prevout.n ).SetNull();
			if ( !m_coinsViewCache->SetCoins(txIn.prevout.hash, coins) )
				return false;
		}

	}

	BOOST_FOREACH( CTransaction const & transaction, _transaction )
	{
		CCoins coins(transaction, 0);
		m_coinsViewCache->SetCoins( transaction.GetHash(), coins);
		std::vector< uint160 > keyIds;
		if ( !retrieveKeyIds( coins, keyIds ) )
			return false;

		BOOST_FOREACH( uint160 const & keyId, keyIds )
		{
			m_addressToCoinsViewCache->setCoins( keyId, transaction.GetHash() );
		}
	}
	m_addressToCoinsViewCache->flush();
	m_coinsViewCache->Flush();

	return true;
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
		if ( !m_coinsViewCache->GetCoins( hash, coins ) )
			return false;
		_coins.push_back( coins );
	}
	return true;
}

void CTransactionRecordManager::addClientTransaction( CTransaction const & _tx )
{
	boost::lock_guard<boost::mutex> lock( m_transactionLock );
	m_transactionPool.push_back( _tx );
}

bool
CTransactionRecordManager::retrieveKeyIds( CCoins const & _coins, std::vector< uint160 > & _keyIds ) const
{

	for (unsigned int i = 0; i < _coins.vout.size(); i++)
	{
		const CTxOut& txout = _coins.vout[i];

		opcodetype opcode;

		std::vector<unsigned char> data;

		CScript::const_iterator pc = txout.scriptPubKey.begin();
		//sanity check
		while( pc != txout.scriptPubKey.end() )
		{
			if (!txout.scriptPubKey.GetOp(pc, opcode, data))
				return false;
		}
		txnouttype type;

		std::vector< std:: vector<unsigned char> > vSolutions;
		if (Solver(txout.scriptPubKey, type, vSolutions) &&
				(type == TX_PUBKEY || type == TX_PUBKEYHASH))
		{
			std::vector<std::vector<unsigned char> >::iterator it = vSolutions.begin();

			while( it != vSolutions.end() )
			{
				if ( type == TX_PUBKEY )
				{
					if ( !txout.IsNull() )
						_keyIds.push_back( Hash160( *it ) );
					break;
				}
				else if( type == TX_PUBKEYHASH )
				{
					if ( !txout.IsNull() )
						_keyIds.push_back( uint160( *it ) );
					break;
				}
				it++;
			}
		}
	}
	return true;
}



void
CTransactionRecordManager::loop()
{
	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock( m_transactionLock );

			if ( !m_transactionPool.empty() )
			{
				common::CActionHandler< TrackerResponses >::getInstance()->executeAction( (common::CAction< TrackerResponses >*)new CValidateTransactionsAction( m_transactionPool ) );
				m_transactionPool.clear();
			}
		}
		MilliSleep(500);
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
