// cloudwu 2012
// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "common/simpleBuddy.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define assert(a) ;
namespace common
{

inline
int
isPowOfTwo(uint32_t x)
{
	return !(x & (x-1));
}

uint32_t
nextPowOfTwo(uint32_t x)
{
	if ( isPowOfTwo(x) )
		return x;

	x |= x>>1;
	x |= x>>2;
	x |= x>>4;
	x |= x>>8;
	x |= x>>16;
	return x+1;
}

CSimpleBuddy::CSimpleBuddy( CSimpleBuddy const & _simpleBuddy )
{
	m_full = _simpleBuddy.m_full;

	memcpy( m_tree, _simpleBuddy.m_tree, ( 1 << ms_buddyBaseLevel ) * 2 );
}

CSimpleBuddy &
CSimpleBuddy::operator=( CSimpleBuddy const & _simpleBuddy )
{
	m_full = _simpleBuddy.m_full;

	memcpy( m_tree, _simpleBuddy.m_tree, ( 1 << ms_buddyBaseLevel ) * 2 );

	return *this;
}

inline
int
CSimpleBuddy::indexOffset(int _index, int _level) const
{
	return ((_index + 1) - (1 << _level)) << (ms_buddyBaseLevel - _level);
}


CSimpleBuddy::CSimpleBuddy()
{
	memset(m_tree , NODE_UNUSED , (1 << ms_buddyBaseLevel) * 2);
}

CSimpleBuddy::~CSimpleBuddy()
{
}

void
CSimpleBuddy::markParent(int _index)
{
	while(1)
	{
		int buddy = _index - 1 + (_index & 1) * 2;
		if (buddy > 0 && (m_tree[buddy] == NODE_USED ||	m_tree[buddy] == NODE_FULL))
		{
			_index = (_index + 1) / 2 - 1;
			m_tree[_index] = NODE_FULL;
		}
		else
		{
			return;
		}
	}
}

int
CSimpleBuddy::buddyAlloc( int _requestedLevel )
{
	int index = 0;
	int level = 0;

	while (index >= 0)
	{
		if (level == _requestedLevel)
		{
			if (m_tree[index] == NODE_UNUSED)
			{
				m_tree[index] = NODE_USED;
				markParent(index);
				return indexOffset(index, level);
			}
		}
		else
		{
			switch (m_tree[index])
			{
				case NODE_USED:
				case NODE_FULL:
					break;
				case NODE_UNUSED:
					m_tree[index] = NODE_SPLIT;
					m_tree[index*2+1] = NODE_UNUSED;
					m_tree[index*2+2] = NODE_UNUSED;
				default:
					index = index * 2 + 1;
					level++;
				continue;
			}
		}
		if (index & 1)
		{
			++index;
			continue;
		}
		while(1)
		{
			level--;
			index = (index+1)/2 -1;

			if (index < 0)
				return -1;

			if (index & 1)
			{
				++index;
				break;
			}
		}
	}
	return -1;
}

void
CSimpleBuddy::combine( int _index)
{
	while(1)
	{
		int buddy = _index - 1 + (_index & 1) * 2;
		if (buddy < 0 || m_tree[buddy] != NODE_UNUSED)
		{
			m_tree[_index] = NODE_UNUSED;
			while (((_index = (_index + 1) / 2 - 1) >= 0) && m_tree[_index] == NODE_FULL)
			{
				m_tree[_index] = NODE_SPLIT;
			}
			return;
		}

		m_tree[_index] = NODE_UNUSED;

		_index = (_index + 1) / 2 - 1;
	}
}

void
CSimpleBuddy::buddyFree(int offset)
{
	assert( offset < (1<< ms_buddyBaseLevel));
	int left = 0;
	int length = 1 << ms_buddyBaseLevel;
	int index = 0;

	while(1)
	{
		switch (m_tree[index])
		{
		case NODE_USED:
			assert(offset == left);
			combine(index);
			return;
		case NODE_UNUSED:
			assert(0);
			return;
		default:
			length /= 2;
			if (offset < left + length)
			{
				index = index * 2 + 1;
			}
			else
			{
				left += length;
				index = index * 2 + 2;
			}
		break;
		}
	}
}

unsigned int
CSimpleBuddy::getBuddyLevel( size_t const _transactionSize )
{
	size_t baseUnit = getBuddySize( ms_buddyBaseLevel );

	unsigned int level = ms_buddyBaseLevel;

	while( ( baseUnit < _transactionSize ) && level )
	{
		level--;
		baseUnit <<=1;
	}

	if ( baseUnit < _transactionSize )
	{
		throw std::exception();
	}

	return level;
}

size_t
CSimpleBuddy::getBuddySize( unsigned int _level )
{
	if ( _level == ( unsigned int )-1 )
		return ms_buddySize;

	size_t baseUnit = ms_buddySize >> ms_buddyBaseLevel;

	return baseUnit << ( ms_buddyBaseLevel - _level );
}



int
CSimpleBuddy::buddySize(int offset) const
{
	assert( offset < (1<< ms_buddyBaseLevel));
	int left = 0;
	int length = 1 << ms_buddyBaseLevel;
	int index = 0;

	while(1)
	{
		switch (m_tree[index])
		{
		case NODE_USED:
			assert(offset == left);
			return length;
		case NODE_UNUSED:
			assert(0);
			return length;
		default:
			length /= 2;
			if (offset < left + length)
			{
				index = index * 2 + 1;
			}
			else
			{
				left += length;
				index = index * 2 + 2;
			}
			break;
		}
	}
}

std::list< int >
CSimpleBuddy::getNotEmptyIndexes( int const _level ) const
{
	int workLevel = _level, index = 1;
	while(workLevel--)
	{
		index*=2;
	}

	std::list< int > notEmpty;
	for ( int i = index - 1;i < 2*index - 1;i++ )
	{
		if ( m_tree[i] == NODE_USED )
		{
			notEmpty.push_back(indexOffset(i, _level) );
		}
	}

	return notEmpty;
}

bool 
CSimpleBuddy::isFull() const
{
	return m_full;
}

}
