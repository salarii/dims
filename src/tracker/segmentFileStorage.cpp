#include "segmentFileStorage.h"

#include "simpleBuddy.h"

#include "helper.h"

namespace self
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

#define assert(a) ;

CDiskBlock::CDiskBlock()
{
}

CDiskBlock::CDiskBlock( CSimpleBuddy const & _simpleBuddy )
	:CSimpleBuddy( _simpleBuddy )
{
	/*  initialise  those  somehow
	m_empty;
	m_blockPosition;
	m_nextSameBucketBlock;*/
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

bool
CSegmentHeader::setNewRecord( unsigned int _bucked, CRecord const & _record )
{
	unsigned int maxNumber =  m_recordsNumber / m_maxBucket;

	for ( unsigned int i = 0; i < maxNumber; i++ )
	{
		CRecord & record = m_records[ _bucked + i * m_maxBucket ];
		if ( !record.m_isEmptySpace && record.m_blockNumber )
		{
			record = _record;
			return true;
		}
	}
	return false;
}

inline
bool
CSegmentHeader::isNextHeader() const
{
	return m_nextHeader;
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


CRecord &
CSegmentHeader::getRecord(unsigned int _index )
{
	return m_records[ _index ];
}

CSegmentFileStorage::CSegmentFileStorage()
: m_recentlyStored(MAX_BUCKET)
{
	fillHeaderBuffor();
}

void
CSegmentFileStorage::includeTransaction( CTransaction const & _transaction )
{
	boost::lock_guard<boost::mutex> lock(m_storeTransLock);
	m_transactionsToStore.push_back( _transaction );
}

void
CSegmentFileStorage::includeTransactions( std::list< CTransaction > const & _transaction )
{
	boost::lock_guard<boost::mutex> lock(m_storeTransLock);
	m_transactionsToStore.insert (m_transactionsToStore.end(),_transaction.begin(),_transaction.end());
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
	return loadSegmentFromFile< CDiskBlock >( _index, ms_segmentFileName, _discBlock );
}

bool
CSegmentFileStorage::getSegmentHeader( unsigned int _index, CSegmentHeader & _segmentHeader )
{
	return loadSegmentFromFile< CSegmentHeader >( _index, ms_headerFileName, _segmentHeader );
}

void
CSegmentFileStorage::saveBlock( unsigned int _index, CSegmentHeader const & _header )
{
	saveSegmentToFile( _index, ms_headerFileName, _header );
}

void
CSegmentFileStorage::saveBlock( unsigned int _index, CDiskBlock const & _block )
{
	saveSegmentToFile( _index, ms_segmentFileName, _block );
}


unsigned int
CSegmentFileStorage::createRecordForBlock( unsigned int _bucket )
{
	std::vector< CSegmentHeader >::iterator iterator = m_headersCache.begin();

	while( iterator != m_headersCache.end() )
	{
		int index = iterator->getIndexForBucket( _bucket );
		if ( -1 != index )
		{
			iterator->setNewRecord( _bucket, CRecord(m_lastSegmentIndex,1));
		}
	}
}

void 
CSegmentFileStorage::flushLoop()
{
	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock(m_cachelock);

			// do something  like  this ???? go over all, save full, and drop
			// do time checking improve  this  by adding logic

			unsigned int storeCandidate = m_recentlyStored.begin()->m_bucked;

			CacheIterators iterator = m_discCache.begin();

			while( iterator != m_discCache.end() )
			{
				if ( m_recentlyStored.find( CStore( iterator->first, 0 ) ) != m_recentlyStored.end() )
				{
					storeCandidate = iterator->first;
					break;
				}
				iterator++;
			}

			ToInclude toInclude = m_discCache.equal_range(storeCandidate);

			int64_t newStoretime = GetTime();

			for ( CacheIterators iterator=toInclude.first; iterator!= toInclude.second; ++iterator )
			{

				if ( iterator->second.m_blockPosition )
				{
					saveBlock( m_lastSegmentIndex, iterator->second );
				}
				else
				{
					m_lastSegmentIndex++;

					createRecordForBlock( storeCandidate );
					saveBlock( m_lastSegmentIndex, iterator->second );
				}
				m_recentlyStored.insert( CStore(newStoretime,storeCandidate) );
			}

			//flush
		}
		{
			boost::lock_guard<boost::mutex> lock(m_headerCacheLock);

			unsigned int index = 0;
			BOOST_FOREACH( CSegmentHeader header, m_headersCache )
			{
				index++;
				saveBlock(index,header);
			}

		}
		//flush

//rebuild merkle and store it, in database
		boost::this_thread::interruption_point();
		MilliSleep(1000*60*3);

	}
}

void
CSegmentFileStorage::loop()
{
	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock(m_storeTransLock);

			BOOST_FOREACH( CTransaction transaction, m_transactionsToStore )
			{
				unsigned int bucket = calculateBucket( transaction.GetHash() );
				unsigned int reqLevel = CSimpleBuddy::getBuddyLevel( transaction.GetSerializeSize(SER_DISK, CTransaction::CURRENT_VERSION) );

				ToInclude toInclude;

				toInclude = m_discCache.equal_range(bucket);

				int index = -1;
				if ( toInclude.first != m_discCache.end() )
				{
					for ( CacheIterators cacheIterator=toInclude.first; cacheIterator!=toInclude.second; ++cacheIterator )
					{
						index = cacheIterator->second.buddyAlloc( reqLevel );
						if ( index == -1 )
							continue;
						else
						{
							CBufferAsStream stream(
								 (char *)cacheIterator->second.translateToAddress( index )
								, cacheIterator->second.getBuddySize( reqLevel )
								, SER_DISK
								, CLIENT_VERSION);

							stream << transaction;

							break;
						}
					}
				}

				if ( index == -1 )
				{
					CDiskBlock discBlock;
					
					index = discBlock.buddyAlloc( reqLevel );

					CBufferAsStream stream(
						  (char *)discBlock.translateToAddress( index )
						, discBlock.getBuddySize( reqLevel )
						, SER_DISK
						, CLIENT_VERSION);

					stream << transaction;

					m_discCache.insert( std::make_pair ( bucket,discBlock) );
				}
			}
		}
		boost::this_thread::interruption_point();
	}
}

void
CSegmentFileStorage::readTransactions( CCoinsViewCache * _coinsViewCache )
{
	boost::lock_guard<boost::mutex> lock(m_headerCacheLock);

	fillHeaderBuffor();

	std::vector< CSegmentHeader >::iterator iterator = m_headersCache.begin();

	for ( int i = 0; i < calculateStoredBlockNumber(); i++)
	{
		CDiskBlock discBlock;

		getBlock( i, discBlock );
		
		int level = CSimpleBuddy::ms_buddyBaseLevel;

		while(level)
		{
			std::list< int > transactionsInd = discBlock.getNotEmptyIndexes( level );

			std::list< int >::iterator iterator = transactionsInd.begin();

			BOOST_FOREACH( int index, transactionsInd )
			{

				CBufferAsStream stream(
					  (char *)discBlock.translateToAddress( index )
					, discBlock.getBuddySize( level )
					, SER_DISK
					, CLIENT_VERSION);

				CTransaction transaction;

				stream >> transaction;
				/* do  something  with  those transactions */
				//	_coinsViewCache->SetCoins(transaction->GetHash(), CCoins( *transaction,*iterator ));

			}
			level--;
		}
	}

}

void
CSegmentFileStorage::eraseTransaction( CTransaction const & _transaction )
{
}

void
CSegmentFileStorage::eraseTransaction( CCoins const & _coins )
{
	int index = -1;

	ToInclude toInclude = m_discCache.equal_range(_coins.m_bucket);

	unsigned short recordId = _coins.nHeight % CSegmentHeader::m_recordsNumber;

	unsigned short cacheId = (_coins.nHeight >> 16)/CSegmentHeader::m_recordsNumber;

	if ( toInclude.first != m_discCache.end() )
	{
		for ( CacheIterators cacheIterator = toInclude.first; cacheIterator!=toInclude.second; ++cacheIterator )
		{
			cacheId--;

			if ( cacheId == 0 )
				cacheIterator->second.buddyFree(_coins.nHeight & 0xff);
			//set  time
		}
	}
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
		getSegmentHeader( 0, header );
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

}
