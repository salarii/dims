// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef FILE_STORAGE_H
#define FILE_STORAGE_H


#include <list>

#include "uint256.h"
#include "serialize.h"

class CTransaction;

class CCoinsViewCache;

namespace self
{

#define BLOCK_SIZE ( 1 << 12 )
#define TRANSACTION_MAX_SIZE ( 1 << 8 )
#define MAX_BUCKET ( 0xff -1 )


typedef unsigned int IndicatorType;
typedef unsigned int CounterType;

//::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
// nBlockSize = ::GetSerializeSize(block, SER_DISK, CLIENT_VERSION);
//tx.GetSerializeSize(SER_NETWORK, CTransaction::CURRENT_VERSION);
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
	bool setNewRecord( unsigned int _bucked, CRecord const & _record );
	

    IMPLEMENT_SERIALIZE
    (
        READWRITE(m_nextHeader);
        READWRITE(FLATDATA(m_records));
        READWRITE(m_headerHash);
    )

	IndicatorType getNextHeader() const;
	void setNextHeader( IndicatorType _nextHeader );

	static unsigned int const getRecordNumber();

	bool givenRecordUsed(unsigned int _index );
private:
	static unsigned int const  m_recordsNumber =  ( BLOCK_SIZE - sizeof( IndicatorType )*2 -  sizeof( uint256 ) )/ sizeof( CRecord );
	static unsigned int const  m_maxBucket = MAX_BUCKET;

	IndicatorType m_nextHeader;
	CRecord m_records[ m_recordsNumber ];

	uint256 m_headerHash;
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
	CHeader * createNewHeader();

	unsigned int calculateBucket( uint256 const & _coinsHash ) const;

	void * getNextFreeBlock();

	void * getBlock( unsigned int _index );

	unsigned int findLastHeader();

	unsigned int calculateBlockIndex( void * _block );

	static const std::string ms_fileName;
private:

	mutable boost::mutex m_lock;

	std::list< CTransaction > m_transactionsToStore;
};



}

#endif // FILE_STORAGE_H
