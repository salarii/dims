// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef SUPPORT_H
#define SUPPORT_H

#include <stdint.h>

namespace common
{

template < class T, class Enum >
void 
serializeEnum( T & _stream, Enum const _enum )
{
	signed int i = (signed int )_enum;
	_stream << i;
};

struct CReadString
{
	template < class S, class T >
	void operator()( S & _stream, T & _object ) const
	{
		_stream >> _object;
	}
};

struct CReadWrite
{
	template < class S, class T >
	void operator()( S & _stream, T const & _object ) const
	{
		_stream << _object;
	}
};

template < class T >
uintptr_t convertToInt( T * _t )
{
	return reinterpret_cast< uintptr_t >( _t );
}

inline
uint256
getRandNumber()
{
	int const ComponentNumber = 8;
	uint32_t number[ ComponentNumber ];

	for( unsigned int i = 0; i < ComponentNumber; ++i )
	{
		number[i] = insecure_rand();
	}

	return *reinterpret_cast< uint256* >( &number[0] );
}

template< class _ComparedType >
struct CLess
{
	bool operator()( _ComparedType const * const _lhs, _ComparedType const * const _rhs ) const
	{
		return _lhs->getKey() < _rhs->getKey();
	}
};

}
#endif
