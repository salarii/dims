#include "segmentFileStorage.h"

#include "simpleBuddy.h"

namespace self
{

//	boost::lock_guard<boost::mutex> lock(m_lock);
CAutoFile file(OpenHeadFile(true), SER_DISK, CLIENT_VERSION);
CBlockHeader header;
file >> header;
block = header;
CSegmentFileStorage::CSegmentFileStorage()
{

}

CAutoFile file(OpenHeadFile(false), SER_DISK, CLIENT_VERSION);
file << merkleBlock.header;

fflush(file);

FileCommit(file);


struct CTransactionRecord
{
	CSimpleBuddy m_simpleBuddy;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(m_simpleBuddy);
    )
};

class CDiskBlock
{
public:
	void removeTransaction();

    IMPLEMENT_SERIALIZE
    (
        READWRITE(m_used);
        READWRITE(m_nextSameBucketBlock);
        READWRITE(m_transactions);
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
CHeader::setNewRecord( unsigned int _bucked, CRecord & const _record )
{
	unsigned int maxNumber =  m_recordsNumber / m_maxBucket;

	for ( int i = 0; i < maxNumber; i++ )
	{
		Record & record = m_records[ _bucked + i * m_maxBucket ];
		if ( !record.emptyEntries && record.m_blockNumber )
		{
			record = _record;
			return true;
		}
	}
	return false;
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

	lastHeader->m_nextHeader = calculateBlockIndex ( nextBlock );

	return static_cast< CHeader * >( nextBlock );
}

void
CSegmentFileStorage::readTransactions( CCoinsViewCache * _coinsViewCache )
{
	FILE* file = OpenDiskFile(CDiskBlockPos(0,0), ms_fileName, true);
	CAutoFile file(OpenHeadFile(true), SER_DISK, CLIENT_VERSION);
// maybe  buffer  needed here

	CHeader header;
	file >> header;
	file >> header;
// it  is not  full  only some  experimental  crap
	for ( unsigned i = 0; i < m_recordsNumber; ++i )
	{
		CTransactionRecord transactionRecord;
		if ( m_blockNumber!= 0)
		file >> transactionRecord;

		int level = transactionRecord.m_simpleBuddy.m_level;

		while(level)
		{
			std::list< int > transactions = transactionRecord.m_simpleBuddy.getNotEmptyIndexes( level );

			level--;

			std::list< int >::iterator iterator = transactions.begin();

			while( iterator != transactions.end() )
			{
				CTransaction * transaction = (CTransaction *)((void*)m_area + *iterator);
				_coinsViewCache->SetCoins(transaction->GetHash(), CCoins( *transaction,*iterator ));

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
	return _coinsHash.GetLow64()%MAX_BUCKET;
}

void *
CSegmentFileStorage::getNextFreeBlock()
{
}

void *
CSegmentFileStorage::getBlock( unsigned int _index )
{

}

unsigned int
CSegmentFileStorage::findLastHeader()
{
}

unsigned int
CSegmentFileStorage::calculateBlockIndex( void * _block )
{
}

}
