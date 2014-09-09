// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ORIGIN_ADDRESS_SCANER_H
#define ORIGIN_ADDRESS_SCANER_H

class CTransaction;

class CTxMemPool;

#include <boost/thread/mutex.hpp>

#include "txmempool.h"
#include <map>
#include <set>

namespace tracker
{

class COriginAddressScanner
{
public:
	void resumeScaning();

	void loop();

	static COriginAddressScanner* getInstance();
// probably wrong, scanner needs to remember balances for  sanity checks, but also has to remember
// when a given transacion was  added this  is  needed because, those  may be included  latter to  downloaded storage
	void addTransaction(long long const _timeStamp, CTransaction const&  _tx);

	void updateTransactionRecord( long long const _timeStamp );
private:
	COriginAddressScanner();

	void lock();

	void unlock();

	void createBaseTransaction(CTransaction const&  _tx);

	void getHeightOfLastScanedBlock();
private:
	static COriginAddressScanner * ms_instance;

	std::list< uint160 > m_BalancesToStore;

	CTxMemPool m_mempool;

	mutable boost::mutex m_lock;

	std::map< long long, CTransaction > m_transactionToProcess;
	std::set< uint256 > m_alreadyProcessed;

//database related
	uint64_t m_currentTime;
	uint64_t m_totalBalance;

	std::vector< std::vector< unsigned char > > m_keys;
	std::vector< uint64_t > m_balances;
};

}

#endif
