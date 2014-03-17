// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef TRANSACTION_RECORD_MANAGER_H
#define TRANSACTION_RECORD_MANAGER_H

#include <vector>


class CTransaction;

class CCoinsViewCache;

class CTxMemPool;

namespace self
{

class CValidationManager;


class CTransactionRecordManager
{
public:
	CTransactionRecordManager();

	~CTransactionRecordManager();

	void addCoinbaseTransaction( CTransaction const & _tx );

	bool checkIfCoinsAvailable( CTransaction const & _tx ) const;
	//time  stamp or  something needed
	//
	bool validateTransactionBundle( std::vector< CTransaction > const & _transaction );

	void loop( std::vector< CTransaction > const & _transaction );
private:
	void synchronize();
	void askForTokens();
private:
	// mutex
	CCoinsViewCache * m_coinsViewCache;

	// mutex
	CTxMemPool * m_memPool;
//	transaction history  section
};


}

#endif // TRANSACTION_RECORD_MANAGER_H
