// Copyright (c) 2014 Ratcoin dev-team
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

class CTransactionRecordManager
{
public:
	~CTransactionRecordManager();

	void addCoinbaseTransaction( CTransaction const & _tx, uint160 const & _keyId  );

	bool checkIfCoinsAvailable( CTransaction const & _tx ) const;
	//time  stamp or  something needed
	//
	bool validateTransactionBundle( std::vector< CTransaction > const & _transaction );

	void loop( std::vector< CTransaction > const & _transaction );

	static CTransactionRecordManager* getInstance( );

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
};


}

#endif // TRANSACTION_RECORD_MANAGER_H
