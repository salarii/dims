#include "segmentFileStorage.h"

#include "simpleBuddy.h"

#include "helper.h"

namespace self
{
/*
split to two files

*/


/*
how  to  append  new  content  to file 
does  the  serialisation  doing i t  right ??


*/


const std::string
CSegmentFileStorage::ms_fileName = "segments";
/*
//	boost::lock_guard<boost::mutex> lock(m_lock);
CAutoFile file(OpenHeadFile(true), SER_DISK, CLIENT_VERSION);
CBlockHeader header;
file >> header;
block = header;

CAutoFile file(OpenHeadFile(false), SER_DISK, CLIENT_VERSION);
file << merkleBlock.header;

fflush(file);

FileCommit(file);
OpenDiskFile(CDiskBlockPos(0,0), "", bool fReadOnly)
*/
#define assert(a) ;
CSegmentFileStorage::CSegmentFileStorage()
{

}


struct CTransactionRecord
{
	static unsigned int const ms_buddyBaseLevel = 16;

	static unsigned int const ms_buddySize = 1 << ( KiloByteShift* 512); // in bytes

	CTransactionRecord();
	
	static unsigned int getBuddyLevel( size_t _transactionSize );

	static size_t getBuddySize( unsigned int  _level );

	void * translateToAddress( unsigned int _index );

	CSimpleBuddy m_simpleBuddy;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(m_simpleBuddy);
    )
};

unsigned int
CTransactionRecord::getBuddyLevel( size_t const _transactionSize )
{
	size_t baseUnit = ms_buddySize >> ms_buddyBaseLevel;

	unsigned int level = ms_buddyBaseLevel;

	while( baseUnit < _transactionSize && level )
	{
		level--;
		baseUnit <<=2;
	}

	if ( baseUnit < _transactionSize )
	{
		throw std::exception();
	}

	return level;
}

size_t
CTransactionRecord::getBuddySize( unsigned int  _level )
{
	size_t baseUnit = ms_buddySize >> ms_buddyBaseLevel;

	return baseUnit << _level;
}

void * 
CTransactionRecord::translateToAddress( unsigned int _index )
{
	size_t baseUnit = ms_buddySize >> ms_buddyBaseLevel;
	return m_simpleBuddy.m_area[ _index ];
}

CTransactionRecord::CTransactionRecord()
	: m_simpleBuddy(ms_buddyBaseLevel)
{
}


class CDiskBlock
{
public:

	CDiskBlock( CTransactionRecord const & _transactionRecord );
	void removeTransaction();

    IMPLEMENT_SERIALIZE
    (
		 READWRITE(m_used);
		 READWRITE(m_nextSameBucketBlock);
		 READWRITE(FLATDATA(m_transactions));
    )
private:
	// make those field be allocated in first buddy of transaction record
	bool m_empty;
	unsigned int m_blockPosition;
	unsigned int m_nextSameBucketBlock;
	CTransactionRecord m_transactionRecord;
};

CDiskBlock( CTransactionRecord const & _transactionRecord )
{
	m_transactionRecord = _transactionRecord;
	/*  initialise  those  somehow
	m_empty;
	m_blockPosition;
	m_nextSameBucketBlock;*/
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
IndicatorType
CSegmentHeader::getNextHeader() const
{
	return m_nextHeader;
}

inline
void
CSegmentHeader::setNextHeader( IndicatorType _nextHeader )
{
	m_nextHeader = _nextHeader;
}

inline
bool
CSegmentHeader::givenRecordUsed(unsigned int _index )
{
	return m_records[_index].m_blockNumber;
}

inline
unsigned int const
CSegmentHeader::getRecordNumber()
{
	return m_recordsNumber;
}

CRecord
CSegmentHeader::getRecord(unsigned int _index ) const
{
	return m_records[ _index ];
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

void 
CSegmentFileStorage::flushLoop()
{
	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock(m_lock);

				ToInclude = m_discCache.equal_range(bucked);

						for ( CacheIterators iterator=ToInclude.first; iterator!=ToInclude.second; ++iterator )

					CTransactionRecord transactionRecord;
					
					// this  code is executed during  flushes
					/*if ( header.setNewRecord( bucket, transactionRecord ) );
					{
						header = createNewHeader();
						header = setNewRecord( bucket, transactionRecord )
					}*/

		boost::this_thread::interruption_point();
	}
}

void
CSegmentFileStorage::loop()
{
	CSegmentHeader header = findLastHeader();

	while(1)
	{
		{
			boost::lock_guard<boost::mutex> lock(m_lock);

			std::list< CTransaction >::iterator iterator = m_transactionsToStore.begin();
			while(iterator != m_transactionsToStore.end())
			{
				unsigned int bucket = calculateBucket( iterator->GetHash() );
				unsigned int reqLevel = CTransactionRecord::getBuddyLevel( iterator->GetSerializeSize(SER_DISK, CTransaction::CURRENT_VERSION) );

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

							stream << *iterator;

							break;
						}
					}
				}

				if ( index == -1 )
				{
					CTransactionRecord transactionRecord;
					
					CBufferAsStream stream(
						  transactionRecord.translateToAddress( index )
						, transactionRecord.getBuddySize( reqLevel )
						, SER_DISK
						, CLIENT_VERSION);

					stream << *iterator;

					m_discCache.insert( std::make_pair ( bucket,transactionRecord); )
				}
				iterator++;
			}
		}
		boost::this_thread::interruption_point();
	}
}

CSegmentHeader *
CSegmentFileStorage::createNewHeader()
{
	void * nextBlock = getNextFreeBlock();
	if ( !nextBlock )
		return 0;

	CSegmentHeader * lastHeader = static_cast< CSegmentHeader * >( getBlock( findLastHeader() ) );

	if ( !lastHeader )
		return 0;

	lastHeader->setNextHeader( calculateBlockIndex ( nextBlock ) );

	return static_cast< CSegmentHeader * >( nextBlock );
}

void
CSegmentFileStorage::readTransactions( CCoinsViewCache * _coinsViewCache )
{
	FILE* file = OpenDiskFile(CDiskBlockPos(0,0), ms_fileName.c_str(), true);


	CSegmentHeader header;
	CBufferedFile blkdat(file, 2*MAX_BLOCK_SIZE, MAX_BLOCK_SIZE+8, SER_DISK, CLIENT_VERSION);
	blkdat >> header;
// it  is not  full  only some  experimental  crap
	for ( unsigned i = 0; i < CSegmentHeader::getRecordNumber(); ++i )
	{
		CTransactionRecord transactionRecord;
		if ( header.givenRecordUsed(i))
			blkdat >> transactionRecord;

		int level = transactionRecord.m_simpleBuddy.m_level;

		while(level)
		{
			std::list< int > transactions = transactionRecord.m_simpleBuddy.getNotEmptyIndexes( level );

			level--;

			std::list< int >::iterator iterator = transactions.begin();

			while( iterator != transactions.end() )
			{
				CTransaction * transaction = (CTransaction *)((void*)transactionRecord.m_simpleBuddy.m_area + *iterator);
			//	_coinsViewCache->SetCoins(transaction->GetHash(), CCoins( *transaction,*iterator ));

				iterator++;
			}

		}
	}
}
// slow
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
CSegmentFileStorage::findLastHeader(CBufferedFile const & blkdat)
{
	CSegmentHeader header = getBlock<CSegmentHeader>( 0 );
	
	while(header.getNextHeader())
	{
		header = getBlock<CSegmentHeader>( header.getNextHeader() )
	}
	return header;
}

unsigned int
CSegmentFileStorage::calculateBlockIndex( void * _block )
{
	return 0;
}

}
