// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

/*
separate headers  and  blocks?
*/
#include <list>
#include <boost/thread.hpp>

#include "uint256.h"
#include "serialize.h"
#include "mruset.h"
#include "simpleBuddy.h"
#include "accessFile.h"

class CTransaction;

class CCoinsViewCache;

class CCoins;

namespace tracker
{

#define BLOCK_SIZE ( 1 << 12 )
#define TRANSACTION_MAX_SIZE ( 1 << 8 )
#define MAX_BUCKET ( 0xf )

typedef unsigned int CounterType;

struct CRecord
{
	CRecord( unsigned int _blockNumber = 0,unsigned char _isEmptySpace = 0 ):m_blockNumber(_blockNumber),m_isEmptySpace(_isEmptySpace){}
	int m_blockNumber;
	unsigned char m_isEmptySpace;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(m_blockNumber);
        READWRITE(m_isEmptySpace);
    )
};


struct CDiskBlock : public CSimpleBuddy
{
public:
	CDiskBlock( CSimpleBuddy const & _simpleBuddy = CSimpleBuddy() );
public:
	unsigned int m_blockPosition;
	int64_t m_lastStoredTime;
	unsigned int m_id;
private:
	static std::map< unsigned int, unsigned int > m_currentLastBlockIds;
};

class CSegmentHeader
{
public:
	CSegmentHeader();

	int getIndexForBucket( unsigned int _bucket ) const;

	bool givenRecordUsed( unsigned int _index ) const;

	unsigned int getUsedRecordNumber() const;

	void setNextHeaderFlag();

	CRecord & getRecord(  unsigned int _bucket, unsigned int _index );

	static unsigned int getNumberOfBucket();

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_nextHeader);
		READWRITE(FLATDATA(m_records));
		READWRITE(m_headerHash);
	)

	static unsigned int const  m_recordsNumber =  ( BLOCK_SIZE - sizeof( unsigned int )*2 -  sizeof( uint256 ) )/ sizeof( CRecord );

private:
	bool isNextHeader() const;

	bool setNewRecord( unsigned int _bucked, CRecord const & _record );

	void increaseUsageRecord( unsigned int _recordId );

	void decreaseUsageRecord( unsigned int _recordId );
private:

	static unsigned int const  m_maxBucket = MAX_BUCKET;

	char m_nextHeader;

	CRecord m_records[ m_recordsNumber ];

	uint256 m_headerHash;

};

struct CLocation
{
	int m_place;
	uint64 m_bucket;
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


	struct CCacheElement
	{
		CCacheElement( CDiskBlock* _discBlock, uint64_t const _location ):m_discBlock( _discBlock ){};

		CDiskBlock* m_discBlock;

		uint64_t const m_location;

		~CCacheElement(){ delete m_discBlock; }

		bool operator<( CCacheElement const & _cacheElement ) const
		{
			return m_location < _cacheElement.m_location;
		}
	};
public:

	void includeTransaction( CTransaction const & _transaction, uint64_t const _timeStamp );

	void eraseTransaction( CTransaction const & _transaction );

	void eraseTransaction( CCoins const & _coins );

	void includeTransactions( std::vector< CTransaction > const & _transaction, uint64_t const _timeStamp );

	void readTransactions( CCoinsViewCache * _coinsViewCache );

	void loop();
	
	void flushLoop();

	static CSegmentFileStorage* getInstance();

	uint64_t getPosition( CTransaction const & _transaction );
private:
	CSegmentFileStorage();

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

	unsigned int findDiscBlockInHeader( uint64_t const _location );

	unsigned int createRecordForBlock( unsigned int _recordIndex );
//risky what _location really is??
	CDiskBlock* getDiscBlock( uint64_t const _location );
// very  risky to  calculate  the same  thing from the same thing
	uint64_t calculateLocationData( uint64_t const _fullPosition );

	// position of given block
	unsigned int getPosition( uint64_t const _fullPosition );

	// size of allocated area in buddy units
	unsigned int getSize( uint64_t const _fullPosition );

	// index of allocated area in buddy
	unsigned int getIndex( uint64_t const _position );

	// bucket of given block
	unsigned int getBucket( uint64_t const _position );
private:
	mutable boost::mutex m_headerCacheLock;
	std::vector< CSegmentHeader > m_headersCache;

	typedef std::multimap< uint64_t, std::vector< CTransaction > > TransactionQueue;

	typedef std::map< uint64_t,CDiskBlock* > UsedBlocks;
private:
	static CSegmentFileStorage * ms_instance;

	static const std::string ms_segmentFileName;

	static const std::string ms_headerFileName;

	mutable boost::mutex m_storeTransLock;

	TransactionQueue m_transactionQueue;

	mutable boost::mutex m_cachelock;
	std::map< uint64_t,CSimpleBuddy* > m_discCache;

	static size_t const m_segmentSize = 1 << KiloByteShift * 512;

	//this is  good for synchronisation concerning short absence, but it should contain rather timestamps and transactions
	mruset< CStore > m_recentlyStored;

	static size_t m_lastSegmentIndex;

	CAccessFile m_accessFile;

	std::vector< CDiskBlock* > m_discBlocksInCurrentFlush;

	UsedBlocks m_usedBlocks;

	mruset< CCacheElement > m_discBlockCache;
	};

}

#endif // FILE_STORAGE_H
