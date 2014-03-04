#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

namespace Self
{



<---header--><transactionblocks_1><transactionblocks_88><><>

<record>
<record>

#define BLOCK_SIZE ( 1 << 12 )
#define TRANSACTION_MAX_SIZE ( 1 << 8 )
#define MAX_BUCKET ( 0xff -1 )


typedef unsigned int IndicatorType;
typedef unsigned int CounterType;
typedef uint256 HashType;

struct Record;

struct CRecord
{
	IndicatorType m_blockNumber;
	CounterType emptyEntries;
};

class CHeader
{
public:
	void increaseUsageRecord( unsigned int _recordId );
	void decreaseUsageRecord( unsigned int _recordId );
	bool setNewRecord( unsigned int _bucked, CRecord & const _record );
	
private:
	IndicatorType m_nextHeader;
	Record m_records[ m_recordsNumber ]

	HashType m_headerHash;

	static const unsigned int m_recordsNumber =  ( BLOCK_SIZE - sizeof( IndicatorType * 2 ) -  sizeof( HashType ) )/ sizeof( CRecord );
	static const unsigned int m_maxBucket = MAX_BUCKET;
};

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


class CDiskBlock
{
public:
	void removeTransaction();
private:
	CounterType m_used;
	IndicatorType nextSameBucketBlock;
	char [ BLOCK_SIZE ];
};




class CFileStorage
{
public:
	void includeTransaction();
	
private:
	void createNewHeader();
	unsigned int calculateBucket( HashType const & _coinsHash );
	void * getNextFreeBlock();
	void * getBlock( unsigned int _index );
	unsigned int findLastHeader();

	unsigned int calculateBlockIndex( void * _block );
};

CHeader * 
CFileStorage::createNewHeader()
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

}

#endif // FILE_STORAGE_H