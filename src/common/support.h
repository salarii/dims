// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef SUPPORT_H
#define SUPPORT_H

#include <stdint.h>
#include "uint256.h"
#include "serialize.h"
#include "version.h"

extern uint32_t insecure_rand(void);

namespace common
{

template < class T >
void
createPayload( T const & _type, std::vector< unsigned char > & _payload )
{
	unsigned int size = ::GetSerializeSize( _type, SER_NETWORK, PROTOCOL_VERSION );
	_payload.resize( size );
	CBufferAsStream stream( (char*)&_payload.front(), size, SER_NETWORK, PROTOCOL_VERSION );
	stream << _type;
}

template < class T >
void
readPayload( std::vector< unsigned char > const & _payload, T & _outMessage )
{
	CBufferAsStream stream( (char*)&_payload.front(), _payload.size(), SER_NETWORK, PROTOCOL_VERSION );
	stream >> _outMessage;
}

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
//for  simple cases
template < class T >
void
castCharVectorToType( std::vector< unsigned char > const & _input, T * _t )
{
	std::copy( _input.begin(), _input.end(), (unsigned char*)_t );
}
//for  simple cases
template < class T >
void
castTypeToCharVector( T const * _t, std::vector< unsigned char > & _output )
{
	_output.resize( sizeof( T ) );
	memcpy( &_output[ 0 ], _t, sizeof( T ) );
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

}
#endif
