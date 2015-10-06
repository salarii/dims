// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "txdb.h"
#include "txmempool.h"

#include "main.h"

#include "common/actionHandler.h"
#include "common/segmentFileStorage.h"
#include "common/supportTransactionsDatabase.h"
#include "common/analyseTransaction.h"

#include "tracker/validateTransactionsAction.h"
#include "tracker/addressToCoins.h"
#include "tracker/transactionRecordManager.h"

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
	m_lastUsedTime = 0;
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
	m_addressToCoinsViewCache = CAddressToCoinsViewCache::getInstance();

	m_supportTransactionsDatabase = common::CSupportTransactionsDatabase::getInstance();
}

CTransactionRecordManager::~CTransactionRecordManager()
{
	delete m_coinsViewCache;
	delete m_memPool;
}

void
CTransactionRecordManager::addCoinbaseTransaction( CTransaction const & _tx, CKeyID const & _keyId  )
{
	CCoins coins(_tx);
	boost::lock_guard<boost::mutex> lock( m_coinsViewLock );

	m_coinsViewCache->SetCoins(_tx.GetHash() , coins);
	m_addressToCoinsViewCache->setCoins( _keyId, _tx.GetHash() );

	std::vector< CKeyID > inputs;
	if ( common::getTransactionInputs( _tx, inputs ) )
		assert( !"problem" );

	m_addressToCoinsViewCache->setTransactionInputs( _tx.GetHash(), inputs );

	CValidationState state;

	// this  is  needed  but does not work like it should, so I commented it out
	//AcceptToMemoryPool(*m_memPool, state, _tx, false, &pfMissingInputs, false);
	m_addressToCoinsViewCache->flush();
	m_coinsViewCache->Flush();
}

void
CTransactionRecordManager::addTransactionToStorage( CTransaction const & _tx )
{
	CTransaction tx( _tx );
	tx.m_location = common::CSegmentFileStorage::getInstance()->assignPosition( _tx );

	m_supportTransactionsDatabase->setTransactionLocation( _tx.GetHash(), tx.m_location );
	common::CSegmentFileStorage::getInstance()->includeTransaction( tx, GetTime() );

	m_supportTransactionsDatabase->flush();
}

bool
CTransactionRecordManager::addTransactionsToStorage( std::vector< CTransaction > const & _transaction )
{
	BOOST_FOREACH( CTransaction const & transaction, _transaction )
	{
		CTransaction tx( transaction );
		tx.m_location = common::CSegmentFileStorage::getInstance()->assignPosition( tx );

		m_supportTransactionsDatabase->setTransactionLocation( tx.GetHash(), tx.m_location );
		common::CSegmentFileStorage::getInstance()->includeTransaction( tx, GetTime() );
	}
	m_supportTransactionsDatabase->flush();
	return true;
}

bool
CTransactionRecordManager::addRetrivedTransactionBundle( std::vector< CTransaction > const & _transaction )
{
	boost::lock_guard<boost::mutex> lock( m_coinsViewLock );

	BOOST_FOREACH( CTransaction const & transaction, _transaction )
	{
		CCoins coins(transaction);

		m_coinsViewCache->SetCoins( transaction.GetHash(), coins);
		std::vector< uint160 > keyIds;

		CAllowedTypes allowedTypes;
		allowedTypes.m_allowed.insert( TX_PUBKEY );
		allowedTypes.m_allowed.insert( TX_PUBKEYHASH );
		for (unsigned int i = 0; i < coins.vout.size(); i++)
		{
			uint160 keyId;

			if ( !retrieveKeyIds( coins.vout[i], keyId, allowedTypes ) )
				return false;

			keyIds.push_back( keyId );
		}

		std::vector< CKeyID > inputs;
		if ( !common::getTransactionInputs( transaction, inputs ) )
			assert( !"problem" );

		m_addressToCoinsViewCache->setTransactionInputs( transaction.GetHash(), inputs );

		BOOST_FOREACH( uint160 const & keyId, keyIds )
		{
			m_addressToCoinsViewCache->setCoins( keyId, transaction.GetHash() );
		}
	}
	m_addressToCoinsViewCache->flush();
	m_coinsViewCache->Flush();

	BOOST_FOREACH( CTransaction const & transaction, _transaction )
	{
		setTransactionToTemporary( transaction );
	}

	return true;
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

			uint160 keyId;
			if ( !retrieveInputIds( txIn, keyId, coins ) )
				return false;

			m_addressToCoinsViewCache->eraseCoins( keyId, txIn.prevout.hash );

			coins.vout.at( txIn.prevout.n ).SetNull();
			if ( !m_coinsViewCache->SetCoins(txIn.prevout.hash, coins) )
				return false;
		}

	}

	BOOST_FOREACH( CTransaction const & transaction, _transaction )
	{
		CCoins coins(transaction);
		m_coinsViewCache->SetCoins( transaction.GetHash(), coins);
		std::vector< uint160 > keyIds;

		CAllowedTypes allowedTypes;
		allowedTypes.m_allowed.insert( TX_PUBKEY );
		allowedTypes.m_allowed.insert( TX_PUBKEYHASH );
		for (unsigned int i = 0; i < coins.vout.size(); i++)
		{
			uint160 keyId;

			if ( !retrieveKeyIds( coins.vout[i], keyId, allowedTypes ) )
				return false;

			keyIds.push_back( keyId );
		}

		std::vector< CKeyID > inputs;
		if ( !common::getTransactionInputs( transaction, inputs ) )
			assert( !"problem" );

		m_addressToCoinsViewCache->setTransactionInputs( transaction.GetHash(), inputs );

		BOOST_FOREACH( uint160 const & keyId, keyIds )
		{
			m_addressToCoinsViewCache->setCoins( keyId, transaction.GetHash() );
		}
	}
	m_addressToCoinsViewCache->flush();
	m_coinsViewCache->Flush();

	BOOST_FOREACH( CTransaction const & transaction, _transaction )
	{
		setTransactionToTemporary( transaction );
	}

	return true;
}

bool
CTransactionRecordManager::checkIfCoinsAvailable( CTransaction const & _tx ) const
{
	return m_coinsViewCache->HaveInputs(_tx);
}

bool
CTransactionRecordManager::validateTransactionBundle( std::vector< CTransaction > const & _transactions, std::vector< unsigned int > & _invalidTransactions )
{
	CValidationState state;
	unsigned int flags = SCRIPT_VERIFY_NOCACHE | SCRIPT_VERIFY_NONE;

	CCheckQueueControl<CScriptCheck> control( &scriptcheckqueue);

	unsigned int transactionIndex = 0;
	BOOST_FOREACH( CTransaction const & transaction, _transactions )
	{
		std::vector<CScriptCheck> vChecks;
		if ( !CheckInputs(transaction, state, *m_coinsViewCache, true, flags, &vChecks ) )
			return false;

		control.Add(vChecks);
		// for now every transaction  separetely

		if ( !control.Wait() )
			_invalidTransactions.push_back( transactionIndex );

		transactionIndex++;
	}
	return true;
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
CTransactionRecordManager::retrieveInputIds( CTxIn const & _txin, uint160 & _keyIds, CCoins const & _prevCoins )
{

	CScript::const_iterator pc = _txin.scriptSig.begin();

	opcodetype opcode;

	std::vector<unsigned char> data;

	while( pc < _txin.scriptSig.end() )
	{
		if (!_txin.scriptSig.GetOp(pc, opcode, data))
			return false;

		if ( data.size() == 33 || data.size() == 65 )
		{
			_keyIds = CPubKey( data ).GetID();

			break;
		}
	}

	CAllowedTypes allowedTypes;
	allowedTypes.m_allowed.insert( TX_PUBKEY );

	retrieveKeyIds( _prevCoins.vout[_txin.prevout.n], _keyIds, allowedTypes );

	return true;
}


bool
CTransactionRecordManager::retrieveKeyIds( CTxOut const & _txout, uint160 & _keyId, CAllowedTypes const & _allowedType ) const
{
		opcodetype opcode;

		std::vector<unsigned char> data;

		CScript::const_iterator pc = _txout.scriptPubKey.begin();
		//sanity check
		while( pc != _txout.scriptPubKey.end() )
		{
			if (!_txout.scriptPubKey.GetOp(pc, opcode, data))
				return false;
		}
		txnouttype type;

		std::vector< std:: vector<unsigned char> > vSolutions;
		if (Solver(_txout.scriptPubKey, type, vSolutions) && _allowedType.isAllowed( type ) )
		{
			std::vector<std::vector<unsigned char> >::iterator it = vSolutions.begin();

			while( it != vSolutions.end() )
			{
				if ( type == TX_PUBKEY )
				{
					if ( !_txout.IsNull() )
						_keyId = Hash160( *it );
					break;
				}
				else if( type == TX_PUBKEYHASH )
				{
					if ( !_txout.IsNull() )
						_keyId = uint160( *it );
					break;
				}
				it++;
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
				common::CActionHandler::getInstance()->executeAction( (common::CAction*)new CValidateTransactionsAction( m_transactionPool ) );
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

uint64_t const UsedTimePeriod = 240;
unsigned int const PeriodsCount = 4;

bool
CTransactionRecordManager::setTransactionToTemporary( CTransaction const & _transaction )
{
	boost::lock_guard<boost::mutex> lock( m_recentTransactionsLock );
	uint64_t time = GetTime();
	if ( time - m_lastUsedTime > UsedTimePeriod )
	{
		if ( m_usedTimes.size() == PeriodsCount )
		{
			std::set< uint64_t >::iterator iterator = m_usedTimes.begin();
			if ( iterator != m_usedTimes.end() )
			{
				m_recentTransactions.erase( *iterator );
				m_usedTimes.erase( iterator );
			}
		}
		m_usedTimes.insert( m_lastUsedTime );
		m_lastUsedTime = time;
	}

	std::map< uint64_t, std::map< uint256, CTransaction > >::iterator iterator =
			m_recentTransactions.find( m_lastUsedTime );

	if ( iterator == m_recentTransactions.end() )
	{
		iterator = m_recentTransactions.insert( std::make_pair( m_lastUsedTime, std::map< uint256, CTransaction >() ) ).first;
	}

	iterator->second.insert( std::make_pair( _transaction.GetHash(), _transaction ) );

	return true;
}

void
CTransactionRecordManager::clearCoinViewDB()
{
	m_coinsViewCache->clearView();
}

void
CTransactionRecordManager::clearAddressToCoinsDatabase()
{
	m_addressToCoinsViewCache->clearView();
}

void
CTransactionRecordManager::clearSupportTransactionsDatabase()
{
	m_supportTransactionsDatabase->clearView();
}

// this is short search, long will be needed at some point
bool
CTransactionRecordManager::getTransaction( uint256 const & _hash, CTransaction & _transaction ) const
{
	boost::lock_guard<boost::mutex> lock( m_recentTransactionsLock );

	std::map< uint64_t, std::map< uint256, CTransaction > >::const_iterator iterator = m_recentTransactions.begin();

	while( iterator != m_recentTransactions.end() )
	{
		std::map< uint256, CTransaction >::const_iterator it = iterator->second.find( _hash );

		if ( it != iterator->second.end() )
		{
			_transaction = it->second;
			return true;
		}

		iterator++;
	}
	return false;
}

}
