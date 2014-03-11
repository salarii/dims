#include "segmentFileStorage.h"

#include "simpleBuddy.h"

#include "main.h"

namespace self
{

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

*/
#define assert(a) ;
CSegmentFileStorage::CSegmentFileStorage()
{

}


struct CTransactionRecord
{
	static unsigned int const ms_defaultLevel = 4;

	CTransactionRecord();

	CSimpleBuddy m_simpleBuddy;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(m_simpleBuddy);
    )

};

CTransactionRecord::CTransactionRecord()
	: m_simpleBuddy(ms_defaultLevel)
{
}


class CDiskBlock
{
public:
	void removeTransaction();

    IMPLEMENT_SERIALIZE
    (
		 READWRITE(m_used);
		 READWRITE(m_nextSameBucketBlock);
		 READWRITE(FLATDATA(m_transactions));
    )
private:
	CounterType m_used;
	IndicatorType m_nextSameBucketBlock;
	char m_transactions[ BLOCK_SIZE ];
};


void
CHeader::increaseUsageRecord( unsigned int _recordId )
{

}

void
CHeader::decreaseUsageRecord( unsigned int _recordId )
{

}

bool
CHeader::setNewRecord( unsigned int _bucked, CRecord const & _record )
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
CHeader::getNextHeader() const
{
	return m_nextHeader;
}

inline
void
CHeader::setNextHeader( IndicatorType _nextHeader )
{
	m_nextHeader = _nextHeader;
}

inline
bool
CHeader::givenRecordUsed(unsigned int _index )
{
	return m_records[_index].m_blockNumber;
}

inline
unsigned int const
CHeader::getRecordNumber()
{
	return m_recordsNumber;
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
CSegmentFileStorage::loop()
{

}



CHeader *
CSegmentFileStorage::createNewHeader()
{
	void * nextBlock = getNextFreeBlock();
	if ( !nextBlock )
		return 0;

	CHeader * lastHeader = static_cast< CHeader * >( getBlock( findLastHeader() ) );

	if ( !lastHeader )
		return 0;

	lastHeader->setNextHeader( calculateBlockIndex ( nextBlock ) );

	return static_cast< CHeader * >( nextBlock );
}

void
CSegmentFileStorage::readTransactions( CCoinsViewCache * _coinsViewCache )
{
	FILE* file = OpenDiskFile(CDiskBlockPos(0,0), ms_fileName.c_str(), true);


	CHeader header;
	CBufferedFile blkdat(file, 2*MAX_BLOCK_SIZE, MAX_BLOCK_SIZE+8, SER_DISK, CLIENT_VERSION);
	blkdat >> header;
// it  is not  full  only some  experimental  crap
	for ( unsigned i = 0; i < CHeader::getRecordNumber(); ++i )
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

void
CSegmentFileStorage::eraseTransaction( CTransaction const & _transaction )
{

}


unsigned int
CSegmentFileStorage::calculateBucket( HashType const & _coinsHash ) const
{
	return 0;//_coinsHash.GetLow64() % MAX_BUCKET;
}

void *
CSegmentFileStorage::getNextFreeBlock()
{
	return 0;
}

void *
CSegmentFileStorage::getBlock( unsigned int _index )
{
	return 0;
}

unsigned int
CSegmentFileStorage::findLastHeader()
{
	return 0;
}

unsigned int
CSegmentFileStorage::calculateBlockIndex( void * _block )
{
	return 0;
}

}
