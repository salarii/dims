#ifndef SUPPORT_TRANSACTIONS_DATABASE_H
#define SUPPORT_TRANSACTIONS_DATABASE_H

#include "leveldbwrapper.h"
#include "uint256.h"

namespace tracker
{

class CTransactionSpecificData
{
public:
	CTransactionSpecificData(size_t _cacheSize, bool fMemory = false, bool fWipe = false);

	bool getTransactionLocation( uint256 const &_hash, uint64_t & _location );
	bool setTransactionLocation( uint256 const &_hash, uint64_t & _location );
	bool eraseTransactionLocation( uint256 const &_hash );
	template< class Batch >
	bool batchWrite( Batch& _batchWrite );

	void clearView();
protected:
	CLevelDBWrapper db;
};

template< class Batch >
bool
CTransactionSpecificData::batchWrite( Batch & _batchWrite )
{
	return db.WriteBatch(_batchWrite.getBatch());
}

class CSupportTransactionsDatabase
{
protected:
	CTransactionSpecificData m_transactionSpecificData;

	std::map<uint256,uint64_t> m_transactionToLocationCache;
public:
	bool getTransactionLocation( uint256 const &_hash, uint64_t & _location );
	bool setTransactionLocation( uint256 const &_hash, uint64_t & _location );
	bool eraseTransactionLocation( uint256 const &_hash );
	bool flush();

	static CSupportTransactionsDatabase* getInstance();

	void clearView();

	~CSupportTransactionsDatabase();
private:
	mutable boost::mutex m_cacheLock;

	CSupportTransactionsDatabase( size_t _cacheSize = 1 << 26):m_transactionSpecificData( _cacheSize){};

	static CSupportTransactionsDatabase * ms_instance;
};

}
#endif // SUPPORT_TRANSACTIONS_DATABASE_HH
