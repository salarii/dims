#include "transactionRecordManager.h"

#include "txdb.h"

#include "txmempool.h"

namespace self
{

CTransactionRecordManager::CTransactionRecordManager()
{
	m_coinsViewCache = new CCoinsViewCache(new CCoinsViewDB );
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
}

bool
CTransactionRecordManager::checkIfCoinsAvailable( CTransaction const & _tx ) const
{
	m_coinsViewCache->HaveInputs(_tx);
}

void
CTransactionRecordManager::validateTransaction( CTransaction const & _tx ) const
{
	CCheckQueueControl<CScriptCheck> control(fScriptChecks && nScriptCheckThreads ? &scriptcheckqueue : NULL);

	std::vector<CScriptCheck> vChecks;
    if (!CheckInputs(tx, state, view, fScriptChecks, flags, nScriptCheckThreads ? &vChecks : NULL))
        return false;
    control.Add(vChecks);

    if (!control.Wait())

}

void
CTransactionRecordManager::validateTransaction( CTransaction const & _tx ) const
{
	CCheckQueueControl<CScriptCheck> control(fScriptChecks && nScriptCheckThreads ? &scriptcheckqueue : NULL);

	std::vector<CScriptCheck> vChecks;
    if (!CheckInputs(tx, state, view, fScriptChecks, flags, nScriptCheckThreads ? &vChecks : NULL))
        return false;
    control.Add(vChecks);

    if (!control.Wait())


}

void
CTransactionRecordManager::handleTransactionBundle()
{
	// expect  vector  of  transactions

	// verify  valid ones

/*
 * signal mechanicks
 * when  signal  came validate  or  recreate  bundle  and  complain in  case  of  problems )
 * ( may relize  that   things  went  wrong, troubleshooting may  try  to  synchronize
 *
 */
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
