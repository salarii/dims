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

#define  KiloByteShift 10

class CTransaction;

class CCoinsViewCache;

namespace self
{

#define BLOCK_SIZE ( 1 << 12 )
#define TRANSACTION_MAX_SIZE ( 1 << 8 )
#define MAX_BUCKET ( 0xff -1 )

typedef unsigned int CounterType;

//::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
// nBlockSize = ::GetSerializeSize(block, SER_DISK, CLIENT_VERSION);
//tx.GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION);
struct CRecord
{

	unsigned int m_blockNumber;
	char m_isEmptySpace;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(m_blockNumber);
        READWRITE(m_isEmptySpace);
    )
};

struct CTransactionRecord : public CSimpleBuddy
{

	CTransactionRecord();


};

class CDiskBlock : public CTransactionRecord
{
public:

	CDiskBlock( CTransactionRecord const & _transactionRecord );
	void removeTransaction();
private:
	unsigned int m_blockPosition;
};


class CSegmentHeader
{
public:
	CSegmentHeader();

	void increaseUsageRecord( unsigned int _recordId );

	void decreaseUsageRecord( unsigned int _recordId );

	bool setNewRecord( unsigned int _bucked, CRecord const & _record );

	void setNextHeaderFlag();

	bool isNextHeader() const;

	bool givenRecordUsed( unsigned int _index ) const;

	unsigned int getUsedRecordNumber() const;

	CRecord getRecord( unsigned int _index );

	static unsigned int const getRecordsNumber();

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_nextHeader);
		READWRITE(FLATDATA(m_records));
		READWRITE(m_headerHash);
	)

private:
	static unsigned int const  m_recordsNumber =  ( BLOCK_SIZE - sizeof( unsigned int )*2 -  sizeof( uint256 ) )/ sizeof( CRecord );
	static unsigned int const  m_maxBucket = MAX_BUCKET;

	char m_nextHeader;

	CRecord m_records[ m_recordsNumber ];

	uint256 m_headerHash;
};

class CSegmentFileStorage
{
public:
	CSegmentFileStorage();

	void includeTransaction( CTransaction const & _transaction );

	void eraseTransaction( CTransaction const & _transaction );

	void includeTransactions( std::list< CTransaction > const & _transaction );

	void readTransactions( CCoinsViewCache * _coinsViewCache );

	void loop();
	
	void flushLoop();
private:
	CSegmentHeader & createNewHeader();

	unsigned int calculateBucket( uint256 const & _coinsHash ) const;

	void * getNextFreeBlock();

	CDiskBlock getBlock( unsigned int _index );

	CSegmentHeader getSegmentHeader( unsigned int _index );

	CSegmentHeader fillHeaderBuffor();

	unsigned int calculateBlockIndex( void * _block );

	unsigned int calculateStoredBlockNumber() const;
private:
	std::vector< CSegmentHeader > m_headersCache;

	typedef std::multimap< unsigned int,CDiskBlock >::iterator CacheIterators;

	typedef std::pair< CacheIterators, CacheIterators > ToInclude;
private:
	static const std::string ms_segmentFileName;

	static const std::string ms_headerFileName;

	mutable boost::mutex m_lock;

	std::list< CTransaction > m_transactionsToStore;

	mutable boost::mutex m_cachelock;
	std::multimap< unsigned int,CDiskBlock > m_discCache;

	static size_t const m_segmentSize = 1 << KiloByteShift * 512;
/*
	FILE* m_headerFile;

	FILE* m_blockFile;
*/
	};

template< class T >
T 
getBlockFromFile( unsigned int _index, std::string const & _fileName )
{
	FILE* file = OpenDiskFile(CDiskBlockPos( 0,sizeof( T ) ), _fileName.c_str(), true);

	unsigned int bufferedSize = 1 << KiloByteShift * 4;
	CBufferedFile blkdat(file, bufferedSize, bufferedSize, SER_DISK, CLIENT_VERSION);

	T block;
	blkdat >> block;

	return block;
}

}

#endif // FILE_STORAGE_H
