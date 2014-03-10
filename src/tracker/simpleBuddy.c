// cloudwu 2012
// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "simpleBuddy.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

namespace self
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

inline
int
CSimpleBuddy::indexOffset(int _index, int _level)
{
	return ((_index + 1) - (1 << _level)) << (m_level - _level);
}


CSimpleBuddy::CSimpleBuddy(int level)
{
	int size = 1 << level;
	struct buddy * self = malloc(sizeof(struct buddy) + sizeof(uint8_t) * (size * 2 - 2));
	m_level = level;
	memset(m_tree , NODE_UNUSED , size*2-1);

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
CSimpleBuddy::buddyAlloc( int _requested )
{
	int size;
	if (_requested==0)
		size = 1;
	else
		size = (int)nextPowOfTwo(_requested);

	int length = 1 << m_level;

	if (size > length)
		return -1;

	int index = 0;
	int level = 0;

	while (index >= 0)
	{
		if (size == length)
		{
			if (m_tree[index] == NODE_UNUSED)
			{
				m_tree[index] = NODE_USED;
				markParent(self, index);
				return indexOffset(index, level, m_level);
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
					length /= 2;
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
			length *= 2;
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
			while (((_index = (_index + 1) / 2 - 1) >= 0) && m_tree[index] == NODE_FULL)
			{
				m_tree[_index] = NODE_SPLIT;
			}
			return;
		}
		_index = (_index + 1) / 2 - 1;
	}
}



void
CSimpleBuddy::buddyFree(int offset)
{
	assert( offset < (1<< self->level));
	int left = 0;
	int length = 1 << self->level;
	int index = 0;

	while(1)
	{
		switch (self->tree[index])
		{
		case NODE_USED:
			assert(offset == left);
			_combine(self, index);
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


int
CSimpleBuddy::buddySize(int offset)
{
	assert( offset < (1<< self->level));
	int left = 0;
	int length = 1 << self->level;
	int index = 0;

	while(1)
	{
		switch (self->tree[index])
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
	for ( unsigned i = index;i < 2*index ;i++ )
	{
		if ( m_tree[i] == NODE_USED )
		{
			notEmpty.push_back(indexOffset(i, _level) );
		}
	}

	return notEmpty;
}

}
