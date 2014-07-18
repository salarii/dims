#ifndef ORIGIN_TRANSACTIONS_DATABASE_H
#define ORIGIN_TRANSACTIONS_DATABASE_H

#include <boost/thread.hpp>


#include "leveldbwrapper.h"
#include "serialize.h"

namespace tracker
{

class COriginTransactionDatabase
{
	CLevelDBWrapper db;
public:
	struct CResourceDescription
	{
		CResourceDescription(): m_timeStamp( 0 ),m_totalBalance( 0 ), m_keyNumber( 0 ){}

		uint64_t m_timeStamp;
		uint64_t m_totalBalance;
		unsigned int m_keyNumber;

		IMPLEMENT_SERIALIZE
		(
			READWRITE( m_timeStamp );
			READWRITE( m_totalBalance );
			READWRITE( m_keyNumber );
		)

	};
public:
	bool storeOriginTransactionsFlush(
			  uint64_t const _timeStamp
			, uint64_t const _totalBalance
			, std::vector< std::vector< unsigned char > > const & _keys
			, std::vector< uint64_t > const & _balances );

	bool getTotalBalance( uint64_t & _timeStamp, uint64_t _totalBalance );

	bool getOriginTransactions( uint64_t const _timeStamp, std::vector< std::vector< unsigned char > > & _keys, std::vector< uint64_t > & _balances );

	static COriginTransactionDatabase* getInstance();
private:
	COriginTransactionDatabase( size_t _cacheSize, bool _memory = false, bool _wipe = false );

	bool setKey( uint64_t const _id, uint64_t const _balance ,std::vector< unsigned char > const & _key );

	bool getKey( uint64_t const _id, uint64_t & _balance ,std::vector< unsigned char > & _key );

	bool getCurrentFlushNumber( unsigned int & _flushNumber );

	bool setCurrentFlushNumber( unsigned int const _flushNumber );

	bool setResourceDescription( unsigned _id, CResourceDescription const & _resourceDescription );

	bool getResourceDescription( unsigned _id, CResourceDescription & _resourceDescription );
private:
	mutable boost::mutex m_lock;

	static COriginTransactionDatabase * ms_instance;
};


}


#endif // ORIGIN_TRANSACTIONS_DATABASE_H
