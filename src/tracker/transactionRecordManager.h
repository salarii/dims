// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRANSACTION_RECORD_MANAGER_H
#define TRANSACTION_RECORD_MANAGER_H

#include <vector>

#include "checkqueue.h"

#include "main.h"

class CTransaction;

class CCoinsViewCache;

class CTxMemPool;

namespace tracker
{

class CValidationManager;
class CAddressToCoinsViewCache;

struct CAllowedTypes
{
	bool isAllowed( txnouttype _type ) const
	{
		return m_allowed.find( _type ) != m_allowed.end();
	}

	std::set< txnouttype > m_allowed;
};

class CTransactionRecordManager
{
public:
	~CTransactionRecordManager();

	static CTransactionRecordManager* getInstance( );

	bool checkIfCoinsAvailable( CTransaction const & _tx ) const;
	//time  stamp or  something needed
	//
	bool validateTransactionBundle( std::vector< CTransaction > const & _transactions, std::vector< unsigned int > & _invalidTransactions );

	void addCoinbaseTransaction( CTransaction const & _tx, uint160 const & _keyId  );

	bool addValidatedTransactionBundle( std::vector< CTransaction > const & _transaction );

	void addTransactionToStorage( CTransaction const & _tx );

	bool addTransactionsToStorage( std::vector< CTransaction > const & _transaction );

	void loop();

	bool getCoins( std::vector< uint256 > const & _transaction,  std::vector< CCoins > & _coins ) const;

	void addClientTransaction( CTransaction const & _tx );

	bool retrieveInputIds( CTxIn const & _txin, uint160 & _keyIds, CCoins const & _prevCoins  );

	bool retrieveKeyIds( CTxOut const & _txout, uint160 & _keyId, CAllowedTypes const & _allowedType ) const;

// included  here when  confirmed
	bool setTransactionToTemporary( CTransaction const & _transaction );

	bool getTransaction( uint256 const & _hash, CTransaction & _transaction ) const;
// additional strages  for  accepted  validated  double   spending  in future
	void clearCoinViewDB();

	void clearAddressToCoinsDatabase();
private:
	void synchronize();

	void askForTokens();

	CTransactionRecordManager();
private:
	static CTransactionRecordManager * ms_instance;

	mutable boost::mutex m_coinsViewLock;
	CCoinsViewCache * m_coinsViewCache;
	CAddressToCoinsViewCache * m_addressToCoinsViewCache;
	// mutex
	CTxMemPool * m_memPool;
//	transaction history  section
	CCheckQueue<CScriptCheck> scriptcheckqueue;

	mutable boost::mutex m_transactionLock;
	std::vector< CTransaction > m_transactionPool;
// recently addmited transactions
	mutable boost::mutex m_recentTransactionsLock;
	std::map< uint64_t, std::map< uint256, CTransaction > > m_recentTransactions;
	uint64_t m_lastUsedTime;
	std::set< uint64_t > m_usedTimes;
};


}

#endif // TRANSACTION_RECORD_MANAGER_H
