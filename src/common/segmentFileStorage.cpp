// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/foreach.hpp>
#include <boost/algorithm/cxx11/any_of.hpp>

#include "util.h"
#include "core.h"
#include "coins.h"

#include "common/blockInfoDatabase.h"
#include "common/supportTransactionsDatabase.h"
#include "common/segmentFileStorage.h"
#include "common/simpleBuddy.h"

namespace common
{

std::string const
CSegmentFileStorage::m_baseDirectory = "//network//";

std::string const
CSegmentFileStorage::m_copyDirectory = "//networkCopy//";

size_t CSegmentFileStorage::m_lastSegmentIndex = 0;

const std::string
CSegmentFileStorage::ms_segmentFileName = "segments";

const std::string
CSegmentFileStorage::ms_headerFileName = "serheaders";

std::map< unsigned int, unsigned int > CDiskBlock::m_currentLastBlockIds;

//#define assert(a) ;

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
CDiskBlock::translateToAddress( unsigned int _index ) const
{
	size_t baseUnit = getBuddySize( ms_buddyBaseLevel );
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
	for( unsigned i = 0;i < m_recordsNumber/MAX_BUCKET; i++)
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
	return !( m_records[_index].m_isEmptySpace );
}

unsigned int
CSegmentHeader::getAllUsedRecordsNumber() const
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


CRecord &
CSegmentHeader::getRecord(unsigned int _bucket, unsigned int _index )
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
: m_synchronizationInProgress( 0 )
, m_recentlyStored(MAX_BUCKET)
, m_lastFlushTime( 0 )
{
	m_transactionQueue = new TransactionQueue;

	CBlockInfoDatabase::getInstance()->loadTimeOfFlush( m_lastFlushTime );

	retriveState();
}

void
CSegmentFileStorage::resetState()
{
	m_headersCache.clear();
	m_alreadyStoredSegments = 0;
	m_lastSegmentIndex = 0;
	m_usedBuddy.clear();
}


void
CSegmentFileStorage::includeTransaction( CTransaction const & _transaction, uint64_t const _timeStamp )
{
	boost::lock_guard<boost::mutex> lock(m_storeTransLock);

	std::vector< CTransaction > transactions;
	transactions.push_back( _transaction );

	m_transactionQueue->insert( std::make_pair( _timeStamp, transactions ) );

	addToRecentlyUsed( _transaction );
}

void
CSegmentFileStorage::addToRecentlyUsed( CTransaction const & _transaction )
{
	boost::lock_guard<boost::mutex> lock(m_locationTobuddy);

	m_locationUsedFromLastUpdate.insert( _transaction.m_location );

	BOOST_FOREACH( CTxIn const & txIn, _transaction.vin )
	{
		uint64_t location;

		if ( !txIn.prevout.IsNull() )
		{
			if ( !CSupportTransactionsDatabase::getInstance()->getTransactionLocation( txIn.prevout.hash, location ) )
				assert( !"serious problem" );

			m_locationUsedFromLastUpdate.insert( CLocation( location ) );
		}
	}
}

void
CSegmentFileStorage::includeTransactions( std::vector< CTransaction > const & _transactions, uint64_t const _timeStamp )
{
	boost::lock_guard<boost::mutex> lock(m_storeTransLock);
	m_transactionQueue->insert( std::make_pair( _timeStamp, _transactions ) );

	BOOST_FOREACH( CTransaction const & transaction, _transactions )
	{
		addToRecentlyUsed( transaction );
	}
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
	if ( m_alreadyStoredSegments <= _index )
		return false;

	boost::filesystem::path path = GetDataDir( common::CDimsParams::getAppType() );
	path += m_baseDirectory + ms_segmentFileName;
	return m_accessFile.loadSegmentFromFile< CDiskBlock >( _index, path.string(), _discBlock );
}

bool
CSegmentFileStorage::getCopyBlock( unsigned int _index, CDiskBlock & _discBlock )
{
	if ( m_alreadyStoredSegments <= _index )
		return false;

	boost::filesystem::path path = GetDataDir( common::CDimsParams::getAppType() );
	path += m_copyDirectory + ms_segmentFileName;
	return m_accessFile.loadSegmentFromFile< CDiskBlock >( _index, path.string(), _discBlock );
}

bool
CSegmentFileStorage::getSegmentHeader( unsigned int _index, CSegmentHeader & _segmentHeader )
{
	boost::filesystem::path path = GetDataDir( common::CDimsParams::getAppType() );
	path += m_baseDirectory + ms_headerFileName;
	return m_accessFile.loadSegmentFromFile< CSegmentHeader >( _index, path.string(), _segmentHeader );
}

bool
CSegmentFileStorage::getCopySegmentHeader( unsigned int _index, CSegmentHeader & _segmentHeader )
{
	boost::filesystem::path path = GetDataDir( common::CDimsParams::getAppType() );
	path += m_copyDirectory + ms_headerFileName;
	return m_accessFile.loadSegmentFromFile< CSegmentHeader >( _index, path.string(), _segmentHeader );
}

void
CSegmentFileStorage::saveBlock( unsigned int _index, CSegmentHeader const & _header )
{
	boost::filesystem::path path = GetDataDir( common::CDimsParams::getAppType() );
	path += m_baseDirectory + ms_headerFileName;
	m_accessFile.saveSegmentToFile( _index, path.string(), _header );
}

void
CSegmentFileStorage::saveBlock( unsigned int _index, CDiskBlock const & _block )
{
	if ( _index >= m_alreadyStoredSegments )
		m_alreadyStoredSegments = _index + 1;
	boost::filesystem::path path = GetDataDir( common::CDimsParams::getAppType() );
	path += m_baseDirectory + ms_segmentFileName;
	m_accessFile.saveSegmentToFile( _index, path.string(), _block );
}

void
CSegmentFileStorage::setRecord( CLocation const & _location, CRecord const & _record )
{
	unsigned int position = getPosition( _location.m_location );

	std::vector< CSegmentHeader >::iterator iterator = m_headersCache.begin();

	std::advance( iterator, position / CSegmentHeader::getNumberOfFullBucketSets() );

	iterator->setNewRecord( getBucket( _location.m_location ), position % CSegmentHeader::getNumberOfFullBucketSets(), _record );
}

void
CSegmentFileStorage::getLocationOfFreeRecordForBucket( unsigned int const _bucket, CLocation & _location )
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

		_location = CLocation( _bucket, index + std::distance( m_headersCache.begin(), iterator )* CSegmentHeader::getNumberOfFullBucketSets() );
	}

	m_headersCache.push_back( CSegmentHeader() );
	_location = CLocation( _bucket, 0 );
}

uint64_t
CSegmentFileStorage::assignPosition( CTransaction const & _transaction )
{
	boost::lock_guard<boost::mutex> lock( m_locationTobuddy );

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
		location = CLocation( bucket, last - 1 );
	}
	else
	{
		m_usedBuddy.insert( std::make_pair( bucket, 1 ) );
		location = CLocation( bucket, 0 );
	}

	m_transactionLocationToBuddy.insert( std::make_pair( location, simpleBuddy ) );

	m_locationUsedFromLastUpdate.insert( location );

	return createFullPosition( last ? last - 1: 0, index, reqLevel, bucket );
}

bool
CSegmentFileStorage::assignBlockNumberInHeaderCache( CLocation const & _location, unsigned int & _bockNumber )
{
	uint64_t location = _location.m_location;

	unsigned int header = getPosition( location )/ CSegmentHeader::getNumberOfFullBucketSets();
	std::vector< CSegmentHeader >::iterator iterator = m_headersCache.begin();

	while ( header >= m_headersCache.size() )
	{
		createNewHeader();

		iterator = m_headersCache.begin();
	}

	std::advance( iterator, header );

	CRecord & record = iterator->getRecord( getBucket( location ), getPosition( location ) % CSegmentHeader::getNumberOfFullBucketSets() );

	if ( record.m_isEmptySpace )
	{
		record.m_isEmptySpace = 0;
		record.m_blockNumber = m_lastSegmentIndex++;
	}
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

uint64_t
CSegmentFileStorage::getTimeStampOfLastFlush()
{
	return m_lastFlushTime;
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
	if ( !assignBlockNumberInHeaderCache( _location, blockNumber ) )
		return 0;

	CDiskBlock * diskBlock = new CDiskBlock;
	if ( blockNumber != (unsigned int)-1 )
		getBlock( blockNumber, *diskBlock );

	return diskBlock;
}

CBufferAsStream
CSegmentFileStorage::createStreamForGivenLocation( uint64_t const _location, std::pair< CLocation, CDiskBlock* > & _usedBlock )
{
	std::map< CLocation,CDiskBlock* >::iterator usedBlock =  m_usedBlocks.find( _location );
	if ( usedBlock == m_usedBlocks.end() )
	{
		mruset< CCacheElement >::iterator cacheIterator = m_discBlockCache.find( CLocation( _location ) );
		if ( cacheIterator != m_discBlockCache.end() )
			usedBlock = m_usedBlocks.insert( std::make_pair( _location, cacheIterator->m_discBlock ) ).first;
		else
			usedBlock = m_usedBlocks.insert( std::make_pair( _location, getDiscBlock( _location ) ) ).first;
	}

	_usedBlock = *usedBlock;

	return CBufferAsStream(
				(char *)usedBlock->second->translateToAddress( getIndex( _location ) )
				, usedBlock->second->getBuddySize( getLevel( _location ) )
				, SER_DISK
				, CLIENT_VERSION);
}

bool
isValid( CTxOut const & _txOut )
{
	return !_txOut.IsNull();
}
// add logic  to limit  max amount of disc flushes. I don't want  to spend to much time in this logic
void 
CSegmentFileStorage::flushLoop()
{
	while(1)
	{
		if ( m_synchronizationInProgress )
		{
			MilliSleep(1000);
			continue;
		}
		{
			TransactionLocationToBuddy processedLocationToBuddy;

			{
				boost::lock_guard<boost::mutex> lock(m_locationTobuddy);

				BOOST_FOREACH( CLocation const & location, m_locationUsedFromLastUpdate )
				{
					assert( m_transactionLocationToBuddy.find( location ) != m_transactionLocationToBuddy.end() );

					processedLocationToBuddy.insert( *m_transactionLocationToBuddy.find( location ) );

				}
			}
			{
				boost::lock_guard<boost::mutex> storeLock(m_storeTransLock);
				m_processedTransactionQueue = m_transactionQueue;

				m_transactionQueue = new TransactionQueue;
			}
			std::multimap< uint64_t, std::vector< CTransaction > >::iterator iterator = m_processedTransactionQueue->begin();

			while( iterator != m_processedTransactionQueue->end() )
			{
				// todo save time stamp in database
				m_lastFlushTime = GetTime();
				CBlockInfoDatabase::getInstance()->storeTimeOfFlush( m_lastFlushTime );

				BOOST_FOREACH( CTransaction const & transaction, iterator->second )
				{
					std::pair< CLocation,CDiskBlock* > usedBlock;

					CBufferAsStream stream( createStreamForGivenLocation( transaction.m_location, usedBlock ) );

					stream << transaction;

					*(CSimpleBuddy*)usedBlock.second = *processedLocationToBuddy.find( transaction.m_location )->second;

					if ( !transaction.IsCoinBase() )
					{
						BOOST_FOREACH( CTxIn const & txIn, transaction.vin )
						{
							uint64_t location;
							if ( !CSupportTransactionsDatabase::getInstance()->getTransactionLocation( txIn.prevout.hash, location ) )
								assert( !"serious problem" );

							CBufferAsStream stream( createStreamForGivenLocation( location, usedBlock ) );

							CTransaction inTransaction;

							stream >> inTransaction;

							inTransaction.vout[ txIn.prevout.n ].SetNull();

							if ( boost::algorithm::any_of( inTransaction.vout.begin(), inTransaction.vout.end(), isValid ) )
							{
								stream.SetPos(0);
								stream << inTransaction;
							}
							else
							{
								CSupportTransactionsDatabase::getInstance()->eraseTransactionLocation( inTransaction.GetHash() );
								usedBlock.second->buddyFree( getIndex( location ) );
								processedLocationToBuddy.find( transaction.m_location )->second->buddyFree( getIndex( location ) );
							}
						}
					}
				}
				iterator++;
			}

			unsigned int blockNumber = 0;


			BOOST_FOREACH( UsedBlocks::value_type const & block, m_usedBlocks )
			{
				if ( assignBlockNumberInHeaderCache( block.first, blockNumber ) )
				{
					saveBlock( blockNumber, *block.second );
				}
			}

			unsigned int headerId = 0;
			BOOST_FOREACH( CSegmentHeader const & header, m_headersCache )
			{
				saveBlock( headerId, header );
				headerId++;
			}
			// reload  mruset

			UsedBlocks nonCacheBlocks;
			BOOST_FOREACH( UsedBlocks::value_type const & block, m_usedBlocks )
			{
				CCacheElement cacheElement( block.second, block.first.m_location );

				if ( m_discBlockCache.find( cacheElement ) == m_discBlockCache.end() )
				{
					nonCacheBlocks.insert( std::make_pair( block.first, block.second ) );
				}
			}

			BOOST_FOREACH( UsedBlocks::value_type const & block, nonCacheBlocks )
			{
				CCacheElement cacheElement( block.second, block.first.m_location );

				if ( m_discBlockCache.find( cacheElement ) == m_discBlockCache.end() )
				{
					m_discBlockCache.insert( cacheElement );
				}
			}

			m_usedBlocks.clear();

			{
				boost::lock_guard<boost::mutex> lock(m_locationTobuddy);
				processedLocationToBuddy.clear();

				m_locationUsedFromLastUpdate.clear();
			}
			delete m_processedTransactionQueue;

			boost::this_thread::interruption_point();
			//MilliSleep(40000);
			MilliSleep(1000);//only for  debug such short period of time
		}
	}
}

// used  during synchronisation
bool
CSegmentFileStorage::readTransactions( CDiskBlock const & _discBlock, std::vector< CTransaction > & _transactions )
{
	boost::lock_guard<boost::mutex> lock(m_headerCacheLock);

	fillHeaderBuffor();

	int level = CSimpleBuddy::ms_buddyBaseLevel;

	while(level)
	{
		std::list< int > transactionsIds = _discBlock.getNotEmptyIndexes( level );

		std::list< int >::iterator iterator = transactionsIds.begin();

		BOOST_FOREACH( int index, transactionsIds )
		{

			CBufferAsStream stream(
						(char *)_discBlock.translateToAddress( index )
						, _discBlock.getBuddySize( level )
						, SER_DISK
						, CLIENT_VERSION);

			CTransaction transaction;

			stream >> transaction;

			_transactions.push_back( transaction );

		}
		level--;
	}
	//sanity
	/*
	std::vector< uint256 > hashes;

	unsigned int bucket = -1;
	BOOST_FOREACH( CTransaction const & transaction, _transactions )
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
*/
	/* should  I play with merkle here??? */
	return true;
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
		if ( getSegmentHeader( 0, header ) )
		{
			m_headersCache.push_back(header);
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
	// acquire lock
	unsigned int blockCnt = 0;
	BOOST_FOREACH( CSegmentHeader const & header, m_headersCache )
	{
		blockCnt += header.getAllUsedRecordsNumber();
	}
	return blockCnt;
}

//maybe not correct
unsigned int
CSegmentFileStorage::getStoredHeaderCount() const
{
	return m_headersCache.size();
}

void
CSegmentFileStorage::retriveState()
{
	fillHeaderBuffor();
	m_alreadyStoredSegments = calculateStoredBlockNumber();
	m_lastSegmentIndex = m_alreadyStoredSegments;

	if ( !m_headersCache.empty() )
	{
		for ( unsigned int bucket = 0; bucket < MAX_BUCKET; ++bucket )
		{
			m_usedBuddy.insert( std::make_pair( bucket, 0 ) );
		}
	}
	CDiskBlock diskBlock;
	BOOST_FOREACH( CSegmentHeader & header, m_headersCache )
	{
		for ( unsigned int setId = 0; setId < header.getNumberOfFullBucketSets(); ++setId )
		{
			for ( unsigned int bucket = 0; bucket < MAX_BUCKET; ++bucket )
			{
				CRecord const & record = header.getRecord( bucket, setId );
				if ( !record.m_isEmptySpace )
				{
					getBlock( record.m_blockNumber, diskBlock );

					m_transactionLocationToBuddy.insert( std::make_pair( CLocation( bucket, m_usedBuddy.at( bucket ) ), new CSimpleBuddy( diskBlock ) ) );
					m_usedBuddy.at( bucket )++;
				}
			}
		}
	}

}

void
CSegmentFileStorage::setSynchronizationInProgress()
{
	m_synchronizationInProgress = 1;
}

void
CSegmentFileStorage::releaseSynchronizationInProgress()
{
	m_synchronizationInProgress = 0;
}

bool
CSegmentFileStorage::setDiscBlock( CDiskBlock const & _discBlock, unsigned int _index, std::vector< CTransaction > & _transactions )
{
	saveBlock( _index, _discBlock );

	if ( !readTransactions( _discBlock, _transactions ) )
		return false;

	return true;
}

bool
CSegmentFileStorage::setDiscBlock( CSegmentHeader const & _segmentHeader, unsigned int _index )
{
	saveBlock( _index, _segmentHeader );
	return true;
}

// should  be  synchronized  in  some  way
void
CSegmentFileStorage::copyFile( std::string _fileName ) const
{
	boost::filesystem::path path = GetDataDir( common::CDimsParams::getAppType() );
	path += m_baseDirectory + _fileName;

	boost::filesystem::path targetPath = GetDataDir( common::CDimsParams::getAppType() );
	targetPath += m_copyDirectory;

	boost::filesystem::create_directory( targetPath );
	boost::filesystem::path file( path );

	targetPath += _fileName;
	boost::filesystem::copy_file( file, targetPath );
}

void
CSegmentFileStorage::copyHeader() const
{
	copyFile( ms_headerFileName );
}

void
CSegmentFileStorage::copyStorage() const
{
	copyFile( ms_segmentFileName );
}

/*
check merkle ?? set  merkle hash in  headers??
 get all transactions by  level from  down  to  top
 count  hases
 put  hashes into  vector
 hash vector of  hashes
 this  is  has of  single  block
 from  this  create  merkle  tree


*/




}
