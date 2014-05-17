// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "segmentFileStorage.h"

#include "simpleBuddy.h"
#include "util.h"
#include <boost/foreach.hpp>
#include "core.h"
#include "coins.h"

namespace tracker
{
/*
split to two files
how  to  append  new  content  to file 
does  the  serialisation  doing i t  right ??

fflush(file);
*/
size_t CSegmentFileStorage::m_lastSegmentIndex = 0;

const std::string
CSegmentFileStorage::ms_segmentFileName = "segments";

const std::string
CSegmentFileStorage::ms_headerFileName = "serheaders";

std::map< unsigned int, unsigned int > CDiskBlock::m_currentLastBlockIds;

#define assert(a) ;



CDiskBlock::CDiskBlock( CSimpleBuddy const & _simpleBuddy )
	:CSimpleBuddy( _simpleBuddy )
{
	for (unsigned int i = 0;i < MAX_BUCKET;i++)
	{
		m_currentLastBlockIds.insert( std::make_pair( i, 0 ) );
	}
	m_area = new unsigned char[ms_buddySize];
}

void *
CDiskBlock::translateToAddress( unsigned int _index )
{
	size_t baseUnit = ms_buddySize >> ms_buddyBaseLevel;
	return (void *)&m_area[ _index * baseUnit ];
}


CSegmentHeader::CSegmentHeader()
	: m_nextHeader(0)
{
}

void
CSegmentHeader::increaseUsageRecord( unsigned int _recordId )
{

}

void
CSegmentHeader::decreaseUsageRecord( unsigned int _recordId )
{

}

CRecord const &
CSegmentHeader::setNewRecord( unsigned int _bucked, unsigned int _position, CRecord const & _record )
{
	assert( _position < m_recordsNumber / m_maxBucket );

	return m_records[ _bucked + _position * m_maxBucket ] = _record;
}

inline
bool
CSegmentHeader::isNextHeader() const
{
	return m_nextHeader;
}

unsigned int
CSegmentHeader::getNumberOfFullBucketSets()
{
	return m_recordsNumber/MAX_BUCKET;
}

int
CSegmentHeader::getIndexForBucket( unsigned int _bucket ) const
{
	for( int i = 0;i < m_recordsNumber/MAX_BUCKET; i++)
	{
		CRecord record = m_records[ i*(m_recordsNumber/MAX_BUCKET) + _bucket ];
		if ( record.m_blockNumber )
			return record.m_blockNumber;
	}
	return -1;
}

inline
bool
CSegmentHeader::givenRecordUsed(unsigned int _index ) const
{
	return m_records[_index].m_blockNumber;
}

unsigned int
CSegmentHeader::getUsedRecordNumber() const
{
	unsigned int usageCnt = 0;
	for ( unsigned i = 0; i < m_recordsNumber; ++i )
	{
		if ( givenRecordUsed(i) )
			usageCnt++;
	}
	return usageCnt;
}

inline
void
CSegmentHeader::setNextHeaderFlag()
{
	m_nextHeader = 1;
}


CRecord const &
CSegmentHeader::getRecord(unsigned int _bucket, unsigned int _index ) const
{
	assert( getNumberOfFullBucketSets() > _index );
	return m_records[ _index * MAX_BUCKET  + _bucket ];

}

CLocation::CLocation( unsigned int _bucket, unsigned int _position )
{
	m_location = CSegmentFileStorage::calculateLocation( _bucket, _position );
}
CLocation::CLocation( uint64_t const & _fullPosition )
{
	m_location = CSegmentFileStorage::calculateLocation( _fullPosition );
}

CSegmentFileStorage * CSegmentFileStorage::ms_instance = NULL;

CSegmentFileStorage*
CSegmentFileStorage::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CSegmentFileStorage();
	};
	return ms_instance;
}


CSegmentFileStorage::CSegmentFileStorage()
: m_recentlyStored(MAX_BUCKET)
{
	fillHeaderBuffor();
}

void
CSegmentFileStorage::includeTransaction( CTransaction const & _transaction, uint64_t const _timeStamp )
{
	boost::lock_guard<boost::mutex> lock(m_storeTransLock);

	std::vector< CTransaction > transactions;
	transactions.push_back( _transaction );

	m_transactionQueue.insert( std::make_pair( _timeStamp, transactions ) );

	m_locationUsedFromLastUpdate.insert( _transaction.m_location );
}

void
CSegmentFileStorage::includeTransactions( std::vector< CTransaction > const & _transactions, uint64_t const _timeStamp )
{
	boost::lock_guard<boost::mutex> lock(m_storeTransLock);
	m_transactionQueue.insert( std::make_pair( _timeStamp, _transactions ) );
}

CSegmentHeader &
CSegmentFileStorage::createNewHeader()
{
	if ( !m_headersCache.empty() )
		m_headersCache.back().setNextHeaderFlag();
	
	m_headersCache.push_back( CSegmentHeader() );
	
	return m_headersCache.back();
}

bool
CSegmentFileStorage::getBlock( unsigned int _index, CDiskBlock & _discBlock )
{
	return m_accessFile.loadSegmentFromFile< CDiskBlock >( _index, ms_segmentFileName, _discBlock );
}

bool
CSegmentFileStorage::getSegmentHeader( unsigned int _index, CSegmentHeader & _segmentHeader )
{
	return m_accessFile.loadSegmentFromFile< CSegmentHeader >( _index, ms_headerFileName, _segmentHeader );
}

void
CSegmentFileStorage::saveBlock( unsigned int _index, CSegmentHeader const & _header )
{
	m_accessFile.saveSegmentToFile( _index, ms_headerFileName, _header );
}

void
CSegmentFileStorage::saveBlock( unsigned int _index, CDiskBlock const & _block )
{
	m_accessFile.saveSegmentToFile( _index, ms_segmentFileName, _block );
}


CRecord const &
CSegmentFileStorage::createRecordForBlock( CLocation const & _location )
{
	uint64_t location = _location.m_location;

	unsigned int header = getPosition( location )/ CSegmentHeader::getNumberOfFullBucketSets();

	std::vector< CSegmentHeader >::iterator iterator = m_headersCache.begin();

	for ( unsigned int i = m_headersCache.size(); i < header; ++i )
	{
			m_headersCache.push_back( CSegmentHeader() );
	}

	std::advance( iterator, header );

	return iterator->setNewRecord( getBucket( location ), getPosition( location ) % CSegmentHeader::getNumberOfFullBucketSets(), CRecord(m_lastSegmentIndex++,1) );
}

CRecord const &
CSegmentFileStorage::getFreeRecordForBucket( unsigned int const _bucket, CLocation & _location )
{
	std::vector< CSegmentHeader >::iterator iterator = m_headersCache.begin();

	while( iterator != m_headersCache.end() )
	{
		int index = iterator->getIndexForBucket( _bucket );
		if ( index == -1 )
		{
			iterator++;
			continue;
		}
		_location = CLocation( _bucket, _index );
		return iterator->getRecord( _bucket, _index );
	}

	m_headersCache.push_back( CSegmentHeader() );
	_location = CLocation( _bucket, 0 );
	return m_headersCache.back().getRecord( _bucket, 0 );
}

uint64_t
CSegmentFileStorage::getPosition( CTransaction const & _transaction )
{
	boost::lock_guard<boost::mutex> lock(m_locationTobuddy);

	unsigned int bucket = calculateBucket( _transaction.GetHash() );
	unsigned int reqLevel = CSimpleBuddy::getBuddyLevel( _transaction.GetSerializeSize(SER_DISK, CTransaction::CURRENT_VERSION) );

	std::map< unsigned int, unsigned int >::iterator iterator = m_usedBuddy.find( bucket );

	int index = -1, last = 0;
	if ( iterator != m_usedBuddy.end() )
	{
		last = iterator->second;

		for ( unsigned i = 0; i < iterator->second; ++i )
		{
			assert( m_transactionLocationToBuddy.find( calculateLocation( bucket, i ) ) != m_transactionLocationToBuddy.end() );
			TransactionLocationToBuddy::iterator buddy = m_transactionLocationToBuddy.find( calculateLocation( bucket, i ) );
			index = buddy->second->buddyAlloc( reqLevel );
			if ( index == -1 )
				continue;

			return createFullPosition( i, index, reqLevel, bucket );
		}
	}

	CSimpleBuddy * simpleBuddy = new CSimpleBuddy;

	index = simpleBuddy->buddyAlloc( reqLevel );

	CLocation location;

	if ( last )
	{
		m_usedBuddy.at( bucket )= ++last;
		m_transactionLocationToBuddy.insert( std::make_pair( CLocation( bucket, last - 1 ), simpleBuddy ) );
	}
	else
	{
		m_usedBuddy.insert( std::make_pair( bucket, 1 ) );
		m_transactionLocationToBuddy.insert( std::make_pair( CLocation( bucket, 0 ), simpleBuddy ) );
	}
	m_locationUsedFromLastUpdate.insert( location );

	return createFullPosition( last ? last - 1: 1, index, reqLevel, bucket );
}

bool
CSegmentFileStorage::findBlockNumberInHeaderCache( CLocation const & _location, unsigned int & _bockNumber ) const
{
	uint64_t location = _location.m_location;

	unsigned int header = getPosition( location )/ CSegmentHeader::getNumberOfFullBucketSets();
	std::vector< CSegmentHeader >::const_iterator iterator = m_headersCache.begin();

	if ( iterator == m_headersCache.end() )
		return false;
	std::advance( iterator , header );

	CRecord record = iterator->getRecord( getBucket( location ), getPosition( location ) % CSegmentHeader::getNumberOfFullBucketSets() );

	_bockNumber = record.m_blockNumber;
	return true;
}

uint64_t
CSegmentFileStorage::calculateLocation( unsigned int _bucket, unsigned int _position )
{
	uint64_t location = _position;
	location <<= 32;
	location = location |_bucket;
	return location;
}

uint64_t
CSegmentFileStorage::calculateLocation( uint64_t const _fullPosition )
{
	return _fullPosition & 0xffffffff000000ff;
}

unsigned int
CSegmentFileStorage::getPosition( uint64_t const _fullPosition )
{
	return _fullPosition >> 32;
}

unsigned int
CSegmentFileStorage::getLevel( uint64_t const _fullPosition )
{
	return ( _fullPosition >> 8 ) & 0xff;
}

unsigned int
CSegmentFileStorage::getIndex( uint64_t const _fullPosition )
{
	return ( _fullPosition >> 16 ) & 0xffff;
}

unsigned int
CSegmentFileStorage::getBucket( uint64_t const _fullPosition )
{
	return _fullPosition & 0xff;
}

uint64_t
CSegmentFileStorage::createFullPosition( unsigned int _blockPosition, unsigned int _index, unsigned int _level, unsigned int _bucket )
{
	uint64_t fullPosition = _blockPosition;
	fullPosition <<= 16;
	fullPosition = fullPosition | _index;
	fullPosition <<= 8;
	fullPosition = fullPosition | _level;
	fullPosition <<= 8;
	fullPosition = fullPosition | _bucket;
	return fullPosition;
}

CDiskBlock*
CSegmentFileStorage::getDiscBlock( uint64_t const _location )
{
	unsigned int blockNumber = 0;
	if ( !findBlockNumberInHeaderCache( _location, blockNumber ) )
		return 0;

	CDiskBlock * diskBlock = new CDiskBlock;
	if ( blockNumber != -1 )
		getBlock( blockNumber, *diskBlock );

	return diskBlock;
}
// add logic  to limit  max amount of disc flushes. I don't want  to spend to much time in this logic

void 
CSegmentFileStorage::flushLoop()
{
	while(1)
	{
		{
			TransactionLocationToBuddy processedLocationToBuddy;

			{
				boost::lock_guard<boost::mutex> lock(m_locationTobuddy);

				BOOST_FOREACH( CLocation const & location, m_locationUsedFromLastUpdate )
				{
					assert( m_transactionLocationToBuddy.find( location ) != m_transactionLocationToBuddy.end() )

					processedLocationToBuddy.insert( *m_transactionLocationToBuddy.find( location ) );

				}
			}
			{
				boost::lock_guard<boost::mutex> lock(m_cachelock);
				m_processedTransactionQueue = m_transactionQueue;

				m_transactionQueue = new TransactionQueue;
			}
			std::multimap< uint64_t, std::vector< CTransaction > >::iterator iterator = m_processedTransactionQueue->begin();

			while( iterator != m_processedTransactionQueue->end() )
			{
				// save time stamp in database
				//
				BOOST_FOREACH( CTransaction const & transaction, iterator->second )
				{
					uint64_t location = transaction.m_location;
					std::map< CLocation,CDiskBlock* >::iterator usedBlock =  m_usedBlocks.find( location );
					if ( usedBlock == m_usedBlocks.end() )
					{
						mruset< CCacheElement >::iterator cacheIterator = m_discBlockCache.find( location );
						if ( cacheIterator != m_discBlockCache.end() )
							usedBlock = m_usedBlocks.insert( std::make_pair( location, cacheIterator->m_discBlock ) ).first;
						else
							usedBlock = m_usedBlocks.insert( std::make_pair( location, getDiscBlock( location ) ) ).first;
					}

					CBufferAsStream stream(
								(char *)usedBlock->second->translateToAddress( getIndex( transaction.m_location ) )
								, usedBlock->second->getBuddySize( getLevel( transaction.m_location ) )
								, SER_DISK
								, CLIENT_VERSION);

					stream << transaction;

					assert( m_discBlockCache.find( location ) != m_discBlockCache.end() );
					*(CSimpleBuddy*)usedBlock->second = *processedLocationToBuddy.find( location )->second;
				}
				iterator++;
			}

			unsigned int blockNumber = 0;
			BOOST_FOREACH( UsedBlocks::value_type const & block, m_usedBlocks)
			{
				if ( findBlockNumberInHeaderCache( block.first, blockNumber ) )
				{
					saveBlock( blockNumber, *block.second );
				}
				else
				{
					CRecord const & record = createRecordForBlock( block.first );

					saveBlock( record.m_blockNumber, *block.second );
				}
			}

			// reload  mruset
			BOOST_FOREACH( UsedBlocks::value_type const & block, m_usedBlocks )
			{
				CCacheElement cacheElement( block.second, block.first.m_location );

				if ( m_discBlockCache.find( cacheElement ) == m_discBlockCache.end() )
				{
					m_discBlockCache.insert( cacheElement );
				}
			}

			m_usedBlocks.clear();

			processedLocationToBuddy.clear();

			m_locationUsedFromLastUpdate.clear();

			delete m_processedTransactionQueue;
			m_accessFile.flush(ms_headerFileName);

			//rebuild merkle and store it, in database
			boost::this_thread::interruption_point();
			//MilliSleep(40000);
			MilliSleep(400);//only for  debug such short period of time
		}
	}
}

// used  during synchronisation
bool
CSegmentFileStorage::readTransactions( CDiskBlock const & _discBlock, std::vector< CTransaction > const & _transactions )
{
	boost::lock_guard<boost::mutex> lock(m_headerCacheLock);

	fillHeaderBuffor();

	int level = CSimpleBuddy::ms_buddyBaseLevel;

	while(level)
	{
		std::list< int > transactionsInd = discBlock.getNotEmptyIndexes( level );

		std::list< int >::iterator iterator = transactionsInd.begin();

		BOOST_FOREACH( int index, transactionsInd )
		{

			CBufferAsStream stream(
						(char *)_discBlock.translateToAddress( index )
						, _discBlock.getBuddySize( level )
						, SER_DISK
						, CLIENT_VERSION);

			CTransaction transaction;

			stream >> transaction;

			_transactions.push_back( transaction );
			/* do  something  with  those transactions */
			//	_coinsViewCache->SetCoins(transaction->GetHash(), CCoins( *transaction,*iterator ));

		}
		level--;
	}
	//sanity
	std::vector< uint256 > hashes;

	unsigned int bucket = -1;
	BOOST_FOREACH( CTransaction const & transaction, transactionsInd )
	{
		hashes.push_back( transaction.GetHash() );
		// should be same bucket
		if ( bucket != -1 )
		{
			if ( calculateBucket( hashes.back() ) != bucket )
				return false;
		}
		else
		{
			bucket = calculateBucket( hashes.back() );
		}
	}

	/* should  I play with merkle here??? */

}

void
CSegmentFileStorage::eraseTransaction( CTransaction const & _transaction )
{
}

void
CSegmentFileStorage::eraseTransaction( CCoins const & _coins )
{
/*	int index = -1;

	ToInclude toInclude;// = m_discCache.equal_range(_coins.m_bucket);

	unsigned short recordId;// = _coins.nHeight % CSegmentHeader::m_recordsNumber;

	unsigned short cacheId;// = (_coins.nHeight >> 16)/CSegmentHeader::m_recordsNumber;

	if ( toInclude.first != m_discCache.end() )
	{
		for ( CacheIterators cacheIterator = toInclude.first; cacheIterator!=toInclude.second; ++cacheIterator )
		{
			cacheId--;

			if ( cacheId == 0 )
				cacheIterator->second->buddyFree(_coins.nHeight & 0xff);
			//set  time
		}
	}*/
}


unsigned int
CSegmentFileStorage::calculateBucket( uint256 const & _coinsHash ) const
{
	return  _coinsHash.GetLow64() % MAX_BUCKET;
}

void *
CSegmentFileStorage::getNextFreeBlock()
{
	return 0;
}

void
CSegmentFileStorage::fillHeaderBuffor()
{
	CSegmentHeader header;

	if ( m_headersCache.size() > 0 )
	{
		header = m_headersCache.back();
	}
	else
	{
		if (! getSegmentHeader( 0, header ) )
		{
			m_headersCache.push_back(header);
			return;
		}
	}

	while(header.isNextHeader())
	{
		getSegmentHeader( m_headersCache.size(), header );
		m_headersCache.push_back(header);
	}
}

unsigned int
CSegmentFileStorage::calculateBlockIndex( void * _block )
{
	return 0;
}


unsigned int
CSegmentFileStorage::calculateStoredBlockNumber() const
{
	unsigned int blockCnt = 0;
	BOOST_FOREACH( CSegmentHeader header, m_headersCache )
	{
		blockCnt += header.getUsedRecordNumber();
	}
}

bool
CSegmentFileStorage::setDiscBlock( CDiskBlock const & _discBlock )
{



	saveBlock( m_lastSegmentIndex, _discBlock );

	getFreeRecordForBucket( bucket, location ).m_blockNumber = m_lastSegmentIndex++;

	// save  last
	m_transactionLocationToBuddy.insert( std::make_pair( location, new CSimpleBuddy( (CSimpleBuddy const & )_discBlock ) ) );
}


/*
synchro  get  time of  last  flush

get balance for  coresponding  time

fetch  all  transaction  packets


		//while (rand() % 3 == 0)
		//    mapNext[pindex->pprev].push_back(pindex);
		peyoad create  this  way
*/



/*
 load CBlocks fro  outside
 rebuild  headers  from  this


check merkle ?? set  merkle hash in  headers??
 get all transactions by  level from  down  to  top
 count  hases
 put  hashes into  vector
 hash vector of  hashes
 this  is  has of  single  block
 from  this  create  merkle  tree


void
CSegmentFileStorage::



*/




}
