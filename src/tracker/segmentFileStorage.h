// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H

namespace self
{

/*

<---header--><transactionblocks_1><transactionblocks_88><><>

<record>
<record>
*/
#define BLOCK_SIZE ( 1 << 12 )
#define TRANSACTION_MAX_SIZE ( 1 << 8 )
#define MAX_BUCKET ( 0xff -1 )


typedef unsigned int IndicatorType;
typedef unsigned int CounterType;
typedef uint256 HashType;


struct CRecord
{

	IndicatorType m_blockNumber;
	char m_isEmptySpace;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(m_blockNumber);
        READWRITE(m_isEmptySpace);
    )
};

class CHeader
{
public:
	void increaseUsageRecord( unsigned int _recordId );
	void decreaseUsageRecord( unsigned int _recordId );
	bool setNewRecord( unsigned int _bucked, CRecord & const _record );
	

    IMPLEMENT_SERIALIZE
    (
        READWRITE(m_nextHeader);
        READWRITE(emptyEntries);
        READWRITE(m_headerHash);
    )
private:


	IndicatorType m_nextHeader;
	Record m_records[ m_recordsNumber ];

	HashType m_headerHash;

	static const unsigned int m_recordsNumber =  ( BLOCK_SIZE - sizeof( IndicatorType * 2 ) -  sizeof( HashType ) )/ sizeof( CRecord );
	static const unsigned int m_maxBucket = MAX_BUCKET;
};

class CSegmentFileStorage
{
public:
	CSegmentFileStorage();

	void includeTransaction( CTransaction const & _transaction );

	void eraseTransaction( CTransaction const & _transaction );

	void includeTransactions( std::list< CTransaction > const & _transaction );

	void readTransactions( CCoinsViewCache * _coinsViewCache );

	void loop();
private:
	void createNewHeader();

	unsigned int calculateBucket( HashType const & _coinsHash ) const;

	void * getNextFreeBlock();

	void * getBlock( unsigned int _index );

	unsigned int findLastHeader();

	unsigned int calculateBlockIndex( void * _block );

	static const std::string ms_fileName = "segments";
private:

	mutable boost::mutex m_lock;

	std::list< CTransaction _transaction > m_transactionsToStore;
};



}

#endif // FILE_STORAGE_H
