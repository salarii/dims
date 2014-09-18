// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

/*
separate headers  and  blocks?
*/
#include <list>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>

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

#define BLOCK_SIZE ( 1 << 10 )
#define TRANSACTION_MAX_SIZE ( 1 << 8 )
#define MAX_BUCKET ( 0x10 ) // not more than 0xff

typedef unsigned int CounterType;

struct CRecord
{
	CRecord( unsigned int _blockNumber = 0,unsigned char _isEmptySpace = 1 ):m_blockNumber(_blockNumber),m_isEmptySpace(_isEmptySpace){}
	unsigned int m_blockNumber;
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
	IMPLEMENT_SERIALIZE
	(
	READWRITE(*(CSimpleBuddy*)this);
	READWRITE(PTRFLATDATA(m_area, ms_buddySize ));
	)

	void * translateToAddress( unsigned int _index ) const;

	~CDiskBlock()
	{
		if ( m_area )
			delete [] m_area;
	}
public:
	unsigned int m_blockPosition;
	int64_t m_lastStoredTime;
	unsigned int m_id;
	unsigned char * m_area;


private:
	static std::map< unsigned int, unsigned int > m_currentLastBlockIds;

};

class CSegmentHeader
{
public:
	CSegmentHeader();

	int getIndexForBucket( unsigned int _bucket ) const;

	bool givenRecordUsed( unsigned int _index ) const;

	unsigned int getAllUsedRecordsNumber() const;

	void setNextHeaderFlag();

	CRecord & getRecord(  unsigned int _bucket, unsigned int _index );

	static unsigned int getNumberOfFullBucketSets();

	IMPLEMENT_SERIALIZE
	(
		READWRITE(m_nextHeader);
		READWRITE(FLATDATA(m_records));
		READWRITE(m_headerHash);
	)
// wrong??
	static unsigned int const  m_recordsNumber =  ( BLOCK_SIZE - sizeof( unsigned int )*2 -  sizeof( uint256 ) )/ sizeof( CRecord );

	CRecord const & setNewRecord( unsigned int _bucked, unsigned int _position, CRecord const & _record );

	bool isNextHeader() const;
private:
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
	uint64_t m_location;
	CLocation(){};

	CLocation( unsigned int _bucket, unsigned int _position );

	CLocation( uint64_t const & _fullPosition );

	bool operator<( CLocation const & _location )const
	{
		return m_location < _location.m_location;
	}
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
		CCacheElement( CLocation const & _location ):m_discBlock(0), m_location(_location){};

		CCacheElement( CDiskBlock* _discBlock, CLocation const _location ):m_discBlock( _discBlock ), m_location(_location){};

		CDiskBlock* m_discBlock;

		CLocation const m_location;

		bool operator<( CCacheElement const & _cacheElement ) const
		{
			return m_location < _cacheElement.m_location;
		}
	};

	class CDiscBlockCache : public mruset< CCacheElement >
	{
	public:
		CDiscBlockCache(): mruset< CCacheElement >(m_maximumSize){}

		std::pair<iterator, bool> insert( CCacheElement const & x)
		{
			if ( nMaxSize && queue.size() == nMaxSize )
			{
				CCacheElement & cacheElement = queue.front();
				delete cacheElement.m_discBlock;
			}
			mruset< CCacheElement >::insert( x );
		}
	private:
		static unsigned int const m_maximumSize = 32;
	};
public:
	// possibly not good enough
	void setSynchronizationInProgress();

	void releaseSynchronizationInProgress();

	bool setDiscBlock( CDiskBlock const & _discBlock, unsigned int _index, std::vector< CTransaction > & _transactions  );

	bool setDiscBlock( CSegmentHeader const & _segmentHeader, unsigned int _index );

	void includeTransaction( CTransaction const & _transaction, uint64_t const _timeStamp );

	void eraseTransaction( CTransaction const & _transaction );

	void eraseTransaction( CCoins const & _coins );

	void includeTransactions( std::vector< CTransaction > const & _transactions, uint64_t const _timeStamp );

	bool readTransactions( CDiskBlock const & _discBlock, std::vector< CTransaction > & _transactions );
	
	void flushLoop();

	static CSegmentFileStorage* getInstance();

	uint64_t assignPosition( CTransaction const & _transaction );

	// very  risky to  calculate  the same  thing from the same thing
	static uint64_t calculateLocation( unsigned int _bucket, unsigned int _position );

	static uint64_t calculateLocation( uint64_t const _fullPosition );

	uint64_t getTimeStampOfLastFlush();

	// position of given block
	static unsigned int getPosition( uint64_t const _fullPosition );

	// level of allocated area in buddy units
	static unsigned int getLevel( uint64_t const _fullPosition );

	// index of allocated area in buddy
	static unsigned int getIndex( uint64_t const _fullPosition );

	// bucket of given block
	static unsigned int getBucket( uint64_t const _fullPosition );

	static uint64_t createFullPosition( unsigned int _blockPosition, unsigned int _index, unsigned int _level, unsigned int _bucket );

	bool getBlock( unsigned int _index, CDiskBlock & _diskBlock );

	void saveBlock( unsigned int _index, CDiskBlock const & _block );

	unsigned int calculateStoredBlockNumber() const;

	unsigned int getStoredHeaderCount() const;

	bool getSegmentHeader( unsigned int _index, CSegmentHeader & _segmentHeader );

	void saveBlock( unsigned int _index, CSegmentHeader const & _header );
private:
	CBufferAsStream
	createStreamForGivenLocation( uint64_t const _location, std::pair< CLocation, CDiskBlock* > & _usedBlock );

	CSegmentFileStorage();

	CSegmentHeader & createNewHeader();

	unsigned int calculateBucket( uint256 const & _coinsHash ) const;

	void * getNextFreeBlock();

	void fillHeaderBuffor();

	unsigned int calculateBlockIndex( void * _block );

	bool assignBlockNumberInHeaderCache( CLocation const & _location, unsigned int & _bockNumber );

	void getLocationOfFreeRecordForBucket( unsigned int const _bucket, CLocation & _location );

	void setRecord( CLocation const & _location, CRecord const & _record );

	void addToRecentlyUsed( CTransaction const & _transaction );
//risky what _location really is??
	CDiskBlock* getDiscBlock( uint64_t const _location );

	void retriveState();
private:
	mutable boost::mutex m_headerCacheLock;

	mutable boost::mutex m_locationTobuddy;

	std::vector< CSegmentHeader > m_headersCache;

	typedef std::multimap< uint64_t, std::vector< CTransaction > > TransactionQueue;

	typedef std::map< CLocation, CDiskBlock* > UsedBlocks;

	typedef std::map< CLocation, CSimpleBuddy* > TransactionLocationToBuddy;

private:
	boost::atomic< int > m_synchronizationInProgress;

	static CSegmentFileStorage * ms_instance;

	static const std::string ms_segmentFileName;

	static const std::string ms_headerFileName;

	mutable boost::mutex m_storeTransLock;

	TransactionQueue * m_transactionQueue;

	TransactionQueue * m_processedTransactionQueue;

	mutable boost::mutex m_cachelock;

	std::map< unsigned int, unsigned int > m_usedBuddy;

	TransactionLocationToBuddy m_transactionLocationToBuddy;

	static size_t const m_segmentSize = 1 << KiloByteShift * 512;

	//this is  good for synchronisation concerning short absence, but it should contain rather timestamps and transactions
	mruset< CStore > m_recentlyStored;

	CAccessFile m_accessFile;

	std::vector< CDiskBlock* > m_discBlocksInCurrentFlush;

	UsedBlocks m_usedBlocks;

	CDiscBlockCache m_discBlockCache;

	std::set< CLocation > m_locationUsedFromLastUpdate;

	uint64_t m_lastFlushTime;

	static std::string const m_baseDirectory;

	static size_t m_lastSegmentIndex;
	// most probably there is  some logic error if  I  have to use this
	unsigned int m_alreadyStoredSegments;
};

}

#endif // FILE_STORAGE_H
