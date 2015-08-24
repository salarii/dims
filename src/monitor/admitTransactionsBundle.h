// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef ADMIT_TRANSACTIONS_BUNDLE_H
#define ADMIT_TRANSACTIONS_BUNDLE_H

#include "core.h"

#include <boost/statechart/state_machine.hpp>

#include "common/action.h"
#include "common/types.h"

namespace monitor
{

struct CWaitForBundle;

// temporary solution
class CPaymentTracking
{
public:
	static CPaymentTracking* getInstance();

	void addTransactionToSearch( uint256 const & _hash, CKeyID const & _keyId );

	void removeTransactionfromSearch( uint256 const & _hash );

	bool isTransactionPresent( uint256 const & _hash );

	void loop();

	void storeTransactions( std::vector< CTransaction > & _transactions )
	{
		boost::lock_guard<boost::mutex> lock( m_mutex );
		m_toSearch.insert( m_toSearch.end(), _transactions.begin(), _transactions.end() );
	}

	void setStoreTransactions( bool _store )
	{
		boost::lock_guard<boost::mutex> lock( m_mutex );
		if ( !_store )
		{
			m_counter--;
			if ( m_counter < 1 )
				m_toSearch.clear();
		}
		m_counter++;
		m_storeTransactions = _store;
	}

	bool getStoreTransactions() const
	{
		boost::lock_guard<boost::mutex> lock( m_mutex );
		return m_storeTransactions;
	}
private:
	CPaymentTracking():m_counter( 0 ){};
private:
	mutable boost::mutex m_mutex;

//	std::map< uint256, CTransaction > m_foundTransactions;

	std::map< uint256, CKeyID > m_searchTransaction;

	std::set< uint256 > m_acceptedTransactons;

	std::vector< CTransaction > m_toSearch;

	static CPaymentTracking * ms_instance;

	bool m_storeTransactions;

	int m_counter;
};

// I base on fact  that  various  nodes  handling the  same transaction  bundle  should  use  the  sema  action  number
class CAdmitTransactionBundle : public common::CAction< common::CMonitorTypes >, public  boost::statechart::state_machine< CAdmitTransactionBundle, CWaitForBundle >
{
public:
	CAdmitTransactionBundle( uint256 const & _actionKey );

	virtual void accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor );

	~CAdmitTransactionBundle(){};
private:
};



}

#endif // ADMIT_TRANSACTIONS_BUNDLE_H
