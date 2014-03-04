#ifndef ORIGIN_ADDRESS_SCANER_H
#define ORIGIN_ADDRESS_SCANER_H

class CTransaction;

class CTxMemPool;

#include <boost/thread/mutex.hpp>

#include "txmempool.h"
#include <map>

namespace Self
{

class COriginAddressScaner
{
public:
	COriginAddressScaner();

	void resumeScaning();

	void addTransaction(long long const _indexHeight, CTransaction const&  _tx);

	void Thread();
private:
	void lock();

	void unlock();

	void createBaseTransaction(CTransaction const&  _tx);

	void saveBalanceToDatabase();

	void createCoinBaseTransaction();

	void getHeightOfLastScanedBlock();


private:

	CTxMemPool m_mempool;

	mutable boost::mutex m_lock;

	std::map< long long, CTransaction > m_transactionToProcess;
};

}




#endif
