// cloudwu 2012
// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef AUTHENTICATION_PROVIDER_H
#define AUTHENTICATION_PROVIDER_H

#include "buddy.h"


#define NODE_UNUSED 0
#define NODE_USED 1
#define NODE_SPLIT 2
#define NODE_FULL 3

namespace self
{

struct CSimpleBuddy
{
public:
	CSimpleBuddy(int _level);

	int buddyAlloc( int _requested );

	void markParent(int _index);

	int indexOffset(int _index, int _level);

	void combine( int _index);

	void buddyFree(int offset);

	int buddySize(int offset);

	std::list< int > getNotEmptyIndexes( int const _level ) const;

    IMPLEMENT_SERIALIZE
    (
        READWRITE(m_level);
        READWRITE(m_tree);
    )
public:
	unsigned int m_level;

	unsigned char m_tree[ms_size];

	unsigned char m_area[ms_size];
};

}
