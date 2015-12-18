// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CHARGE_REGISTER_H
#define CHARGE_REGISTER_H

#include "uint256.h"
#include "core.h"

namespace monitor
{

struct CTransactionCheck
{
	CTransactionCheck( CKeyID const & _keyId, unsigned int _price )
		: m_keyId( _keyId )
		, m_price( _price )
	{}

	CKeyID m_keyId;
	unsigned int m_price;
};

class CChargeRegister
{
public:
	static CChargeRegister* getInstance();

	void addTransactionToSearch( uint256 const & _hash, CTransactionCheck const & _keyId );

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
	CChargeRegister():m_counter( 0 ){};
private:
	mutable boost::mutex m_mutex;

	std::map< uint256, CTransactionCheck > m_searchTransaction;

	std::set< uint256 > m_acceptedTransactons;

	std::vector< CTransaction > m_toSearch;

	static CChargeRegister * ms_instance;

	bool m_storeTransactions;

	int m_counter;
};


}

#endif
