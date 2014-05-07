// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef SUPPORT_H
#define SUPPORT_H

namespace  common
{

template < class T, class Enum >
void 
serializeEnum( T & _stream, Enum const _enum )
{
	signed int i = _enum;
	_stream << _enum;
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

/*
template < class S >
void
writeTrackerInfo( S & _stream ,std::vector< std::string > const, std::vector< TrackerInfo::Enum >const )
{
}
*/
}
#endif
