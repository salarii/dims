// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef EXPIRE_REGISTER_H
#define EXPIRE_REGISTER_H

#include "serialize.h"
#include "core.h"

namespace common
{

class CExpireRegister
{
public:
	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_transactions);
		READWRITE(m_nextCleanup);
	)

	void loop();

	void addTransaction( int64_t _time, uint256 const & _transactionHash );

	virtual void startCleanupAction() = 0;

	static CExpireRegister* getInstance();
private:
	CExpireRegister();

private:
	mutable boost::mutex m_mutex;

	std::multimap< int64_t, uint256 > m_transactions;

	static int64_t const m_sleepTime;

	int64_t m_nextCleanup;

	std::vector< CTransaction > m_recentTransactions;

	static CExpireRegister * ms_instance;
};

}

#endif // EXPIRE_REGISTER_H
