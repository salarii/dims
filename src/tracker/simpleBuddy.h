// cloudwu 2012
// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SIMPLE_BUDDY_H
#define SIMPLE_BUDDY_H


#define NODE_UNUSED 0
#define NODE_USED 1
#define NODE_SPLIT 2
#define NODE_FULL 3

#include <list>
#include "serialize.h"

#define  KiloByteShift 10

namespace self
{

struct CSimpleBuddy
{
public:
	CSimpleBuddy();

	int buddyAlloc( int _requested );

	void buddyFree(int offset);

	int buddySize(int offset) const;

	std::list< int > getNotEmptyIndexes( int const _level ) const;

	bool isFull() const;

	static unsigned int getBuddyLevel( size_t _transactionSize );

	void * translateToAddress( unsigned int _index );

	static size_t getBuddySize( unsigned int  _level );

    IMPLEMENT_SERIALIZE
    (
        READWRITE(FLATDATA(m_tree));
        READWRITE(FLATDATA(m_area));
    )
public:
	static unsigned int const ms_buddyBaseLevel = 16;

	static unsigned int const ms_buddySize = 1 << ( KiloByteShift* 512); // in bytes
private:

	void markParent(int _index);

	int indexOffset(int _index, int _level) const;

	void combine( int _index);
private:
	char m_full;

	static const int m_level = 6;

	unsigned char m_tree[m_level];

	unsigned char m_area[m_level];
};

}

#endif
