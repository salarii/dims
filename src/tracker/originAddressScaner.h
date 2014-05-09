// Copyright (c) 2014 Ratcoin dev-team
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

class COriginAddressScaner
{
public:
	void resumeScaning();

	void addTransaction(long long const _indexHeight, CTransaction const&  _tx);

	void loop();

	static COriginAddressScaner* getInstance( );
private:
	COriginAddressScaner();

	void lock();

	void unlock();

	void createBaseTransaction(CTransaction const&  _tx);

	void saveBalanceToDatabase();

	void createCoinBaseTransaction();

	void getHeightOfLastScanedBlock();
private:
	static COriginAddressScaner * ms_instance;

	std::list< uint160 > m_BalancesToStore;

	CTxMemPool m_mempool;

	mutable boost::mutex m_lock;

	std::map< long long, CTransaction > m_transactionToProcess;
	std::set< uint256 > m_alreadyProcessed;
};

}




#endif
