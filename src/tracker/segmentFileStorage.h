// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

/*
separate headers  and  blocks?
*/
#include <list>

#include "uint256.h"
#include "serialize.h"

#include "mruset.h"

#include "simpleBuddy.h"

#include "uglyTestHelper.h"

#include "version.h"

class CTransaction;

class CCoinsViewCache;

class CCoins;

namespace self
{

#define BLOCK_SIZE ( 1 << 12 )
#define TRANSACTION_MAX_SIZE ( 1 << 8 )
#define MAX_BUCKET ( 0xf )

typedef unsigned int CounterType;

//::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
// nBlockSize = ::GetSerializeSize(block, SER_DISK, CLIENT_VERSION);
//tx.GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION);
struct CRecord
{
	CRecord( unsigned int _blockNumber = 0,unsigned char _isEmptySpace = 0 ):m_blockNumber(_blockNumber),m_isEmptySpace(_isEmptySpace){}
	unsigned int m_blockNumber;
	unsigned char m_isEmptySpace;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(m_blockNumber);
        READWRITE(m_isEmptySpace);
    )
};


class CDiskBlock : public CSimpleBuddy
{
public:
	CDiskBlock();
	CDiskBlock( CSimpleBuddy const & _simpleBuddy );
public:
	unsigned int m_blockPosition;
	int64_t m_lastStoredTime;
};

class CSegmentHeader
{
public:
	CSegmentHeader();

	void increaseUsageRecord( unsigned int _recordId );

	void decreaseUsageRecord( unsigned int _recordId );

	bool setNewRecord( unsigned int _bucked, CRecord const & _record );

	bool givenRecordUsed( unsigned int _index ) const;

	int getIndexForBucket( unsigned int _bucket ) const;

	bool isNextHeader() const;

	unsigned int getUsedRecordNumber() const;

	void setNextHeaderFlag();

	CRecord & getRecord( unsigned int _index );

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_nextHeader);
		READWRITE(FLATDATA(m_records));
		READWRITE(m_headerHash);
	)

	static unsigned int const  m_recordsNumber =  ( BLOCK_SIZE - sizeof( unsigned int )*2 -  sizeof( uint256 ) )/ sizeof( CRecord );

private:
//put here private  functions
private:

	static unsigned int const  m_maxBucket = MAX_BUCKET;

	char m_nextHeader;

	CRecord m_records[ m_recordsNumber ];

	uint256 m_headerHash;
};

class CSegmentFileStorage
{
public:
	struct CStore
	{
		CStore( int64_t _time, unsigned int _bucked ):m_time(_time),m_bucked(_bucked){}
		int64_t m_time;
		unsigned int m_bucked;
	    bool operator==( CStore const & _store) const { return m_bucked == _store.m_bucked; }
	    bool operator<( CStore const & _store)const { return m_time < _store.m_time; }
	};

public:
	CSegmentFileStorage();

	void includeTransaction( CTransaction const & _transaction );

	void eraseTransaction( CTransaction const & _transaction );

	void eraseTransaction( CCoins const & _coins );

	void includeTransactions( std::list< CTransaction > const & _transaction );

	void readTransactions( CCoinsViewCache * _coinsViewCache );

	void loop();
	
	void flushLoop();
private:
	CSegmentHeader & createNewHeader();

	unsigned int calculateBucket( uint256 const & _coinsHash ) const;

	void * getNextFreeBlock();

	bool getBlock( unsigned int _index, CDiskBlock & _diskBlock );

	bool getSegmentHeader( unsigned int _index, CSegmentHeader & _segmentHeader );

	void saveBlock( unsigned int _index, CSegmentHeader const & _header );

	void saveBlock( unsigned int _index, CDiskBlock const & _block );

	void fillHeaderBuffor();

	unsigned int calculateBlockIndex( void * _block );

	unsigned int calculateStoredBlockNumber() const;

	unsigned int createRecordForBlock( unsigned int _recordIndex );
private:
	mutable boost::mutex m_headerCacheLock;
	std::vector< CSegmentHeader > m_headersCache;

	typedef std::multimap< unsigned int,CDiskBlock >::iterator CacheIterators;

	typedef std::pair< CacheIterators, CacheIterators > ToInclude;
private:
	static const std::string ms_segmentFileName;

	static const std::string ms_headerFileName;

	mutable boost::mutex m_storeTransLock;

	std::list< CTransaction > m_transactionsToStore;

	mutable boost::mutex m_cachelock;
	std::multimap< unsigned int,CDiskBlock > m_discCache;

	static size_t const m_segmentSize = 1 << KiloByteShift * 512;

	mruset< CStore > m_recentlyStored;

	static size_t m_lastSegmentIndex;

/*
	FILE* m_headerFile;

	FILE* m_blockFile;
*/
	};

template< class T >
bool
loadSegmentFromFile( unsigned int _index, std::string const & _fileName, T & _t )
{
	if (!FileExist(_fileName.c_str()))
		return false;

	FILE* file = OpenDiskFile(ugly::CDiskBlockPos( 0,sizeof( T )* _index ), _fileName.c_str(), true);

	unsigned int bufferedSize = 1 << KiloByteShift * 4;
	CBufferedFile blkdat(file, bufferedSize, bufferedSize, SER_DISK, CLIENT_VERSION);

	blkdat >> _t;

	return true;
}

template< class T >
void
saveSegmentToFile( unsigned int _index, std::string const & _fileName, T const & _block )
{
	FILE* file = OpenDiskFile(ugly::CDiskBlockPos( 0,sizeof( T )*_index ), _fileName.c_str(), true);
	CAutoFile autoFile(file, SER_DISK, CLIENT_VERSION);
	autoFile << _block;
}

}

#endif // FILE_STORAGE_H
