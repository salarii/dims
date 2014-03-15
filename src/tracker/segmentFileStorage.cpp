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
const std::string
CSegmentFileStorage::ms_segmentFileName = "segments";

const std::string
CSegmentFileStorage::ms_headerFileName = "serheaders";

#define assert(a) ;

CTransactionRecord::CTransactionRecord()
	: m_simpleBuddy(ms_buddyBaseLevel)
{
}

CDiskBlock( CTransactionRecord const & _transactionRecord )
{
	m_transactionRecord = _transactionRecord;
	/*  initialise  those  somehow
	m_empty;
	m_blockPosition;
	m_nextSameBucketBlock;*/
}

CSegmentHeader::CSegmentHeader()
	: m_nextHeader(0);
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
void
CSegmentHeader::setNextHeaderFlag()
{
	m_nextHeader = 1;
}

inline
bool
CSegmentHeader::givenRecordUsed(unsigned int _index )
{
	return m_records[_index].m_blockNumber;
}

unsigned int getUsedRecordNumber() const
{
	unsigned int usageCnt = 0;
	for ( unsigned i = 0; i < CSegmentHeader::getRecordNumber(); ++i )
	{
		if ( givenRecordUsed(i) )
			usageCnt++
	}
	return usageCnt;
}

CRecord
CSegmentHeader::getRecord(unsigned int _index ) const
{
	return m_records[ _index ];
}

CSegmentFileStorage::CSegmentFileStorage()
: m_recentlyStored(4)
{
}

void
CSegmentFileStorage::includeTransaction( CTransaction const & _transaction )
{
	boost::lock_guard<boost::mutex> lock(m_lock);
	m_transactionsToStore.push_back( _transaction );
}

void
CSegmentFileStorage::includeTransactions( std::list< CTransaction > const & _transaction )
{
	boost::lock_guard<boost::mutex> lock(m_lock);
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

CDiskBlock 
CSegmentFileStorage::getBlock( unsigned int _index )
{
	return getBlockFromFile( _index, ms_segmentFileName );
}

CSegmentHeader 
CSegmentFileStorage::getSegmentHeader( unsigned int _index )
{
	return getBlockFromFile( _index, ms_headerFileName );
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
			iterator->setNewRecord( _bucked, CRecord(m_lastSegmentIndex,1));
		}
	}
}

void 
CSegmentFileStorage::flushLoop()
{
	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock(m_lock);

			// do something  like  this ???? go over all, save full, and drop
			// do time checking improve  this  by adding logic

			unsigned int storeCandidate = m_recentlyStored.begin()->m_bucked;

			CacheIterators iterator = m_discCache.begin();

			while( iterator != iterator.end() )
			{
				if ( !m_recentlyStored.find( CStore( iterator->first, 0 ) );
				{
					storeCandidate = iterator->first;
					break;
				}
				iterator++;
			}

			ToInclude = m_discCache.equal_range(storeCandidate);

			int64_t newStoretime = GetTime();

			for ( CacheIterators iterator=ToInclude.first; iterator!=ToInclude.second; ++iterator )
			{

				if ( iterator->m_blockPosition )
				{
					setBlock( m_lastSegmentIndex, *iterator );
				}
				else
				{
					m_lastSegmentIndex++;

					createRecordForBlock( storeCandidate );
					setBlock( m_lastSegmentIndex, *iterator );
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
	fillHeaderBuffor();

	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock(m_lock);

			BOOST_FOREACH( CTransaction transaction, m_transactionsToStore )
			{
				unsigned int bucket = calculateBucket( transaction.GetHash() );
				unsigned int reqLevel = CSimpleBuddy::getBuddyLevel( transaction.GetSerializeSize(SER_DISK, CTransaction::CURRENT_VERSION) );

				ToInclude toInclude;

				toInclude = m_discCache.equal_range(bucked);

				int index = -1;
				if ( toInclude.first != m_discCache.end() )
				{
					for ( CacheIterators cacheIterator=toInclude.first; cacheIterator!=toInclude.second; ++cacheIterator )
					{
						index = cacheIterator->buddyAlloc( reqLevel );
						if ( index == -1 )
							continue;
						else
						{
							CBufferAsStream stream(
								 cacheIterator->translateToAddress( index )
								, cacheIterator->getBuddySize( reqLevel )
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
					
					CBufferAsStream stream(
						  discBlock.translateToAddress( index )
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
	fillHeaderBuffor();

	std::vector< CSegmentHeader > iterator = m_headersCache.begin();

	for ( int i = 0; i < calculateStoredBlockNumber(); i++)
	{
		CDiskBlock discBlock;

		discBlock = getBlock( i );
		
		int level = ms_buddyBaseLevel;

		while(level)
		{
			std::list< int > transactionsInd = discBlock.getNotEmptyIndexes( level );

			std::list< int >::iterator iterator = transactionsInd.begin();

			BOOST_FOREACH( int index, transactionsInd )
			{

				CBufferAsStream stream(
					  discBlock->translateToAddress( index )
					, discBlock->getBuddySize( level )
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
	if ( _coins.nHeight != 0 )
	{
		CSegmentHeader header = getBlock<CSegmentHeader>( 0 );

		unsigned int recordNumberInHeader = CSegmentHeader::getRecordSetNumber();

		unsigned short recordNumber = _coins.nHeight >> 16;

		CRecord record = findGivenHeader( recordNumber/recordNumberInHeader ).getRecord(recordNumberInHeader*recordNumber%recordNumberInHeader + _coins.m_bucket );

		CTransactionRecord blockRecord = getBlock<CTransactionRecord>( record.m_blockNumber );

		blockRecord.buddyFree(_coins.nHeight & 0xff);

		m_discCache.insert( _coins.m_bucket, blockRecord);
	}
	else
	{
		ToInclude toInclude;

		toInclude = m_discCache.equal_range(bucked);

		int index = -1;
		if ( toInclude.first != m_discCache.end() )
		{
			for ( CacheIterators cacheIterator=toInclude.first; cacheIterator!=toInclude.second; ++cacheIterator )
			{
				/*
				find  using  
				
				*/

				index = cacheIterator->buddyAlloc( reqLevel );
			}
		}

	}
}

CSegmentHeader 
CSegmentFileStorage::findGivenHeader( unsigned int _index )
{
	std::vector< unsigned int >
	m_headersPositions.size() > _headerIndex

	return m_headersPositions

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

CSegmentHeader
CSegmentFileStorage::fillHeaderBuffor()
{
	CSegmentHeader header;

	if ( m_headersCache.size() > 0 )
	{
		header = &m_headersCache.back();
	}
	else
	{
		header = getSegmentHeader( 0 );
	}

	while(header.isNextHeader())
	{
		header = getBlock<CSegmentHeader>( m_headersCache.size() );
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
