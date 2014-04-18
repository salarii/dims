// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef SUPPORT_H
#define SUPPORT_H

#include "sendInfoRequestAction.h"

namespace  node 
{

template < class T, class Enum >
void 
serializeEnum( T & _stream, Enum const _enum )
{
	int i = _enum;
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

template < class S, class T, class F >
void
readWriteTrackerInfo( S & _stream,T & _trackerStats, F const & _operation, std::vector< TrackerInfo::Enum >const & _info )
{
	BOOST_FOREACH( TrackerInfo::Enum const info, _info )
	{
		switch ( info )
		{
		case TrackerInfo::Ip:
			F()( _stream, _trackerStats.m_ip );
			break;
		case TrackerInfo::Price:
			F()( _stream, _trackerStats.m_price );
			break;
		case TrackerInfo::Rating:
			F()( _stream, _trackerStats.m_reputation );
			break;
        case TrackerInfo::PublicKey:
			F()( _stream, _trackerStats.m_publicKey );
        case TrackerInfo::Port:
            F()( _stream, _trackerStats.m_port );
            break;
		default:
			break;
		}
	}
}

template < class S >
void
readTrackerInfo( S & _stream ,CTrackerStats & _trackerStats, std::vector< TrackerInfo::Enum >const & _info )
{
	readWriteTrackerInfo( _stream,_trackerStats, CReadString(), _info );
}

template < class S >
void
writeTrackerInfo( S & _stream ,CTrackerStats const & _trackerStats, std::vector< TrackerInfo::Enum >const & _info )
{
	readWriteTrackerInfo( _stream,_trackerStats, CReadWrite(), _info );
}
/*
template < class S >
void
writeTrackerInfo( S & _stream ,std::vector< std::string > const, std::vector< TrackerInfo::Enum >const )
{
}
*/
}
#endif
