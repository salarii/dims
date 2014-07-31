// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef HELPER_H
#define HELPER_H

#include "sendInfoRequestAction.h"
#include "common/support.h"

namespace  client
{

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
readTrackerInfo( S & _stream ,common::CTrackerStats & _trackerStats, std::vector< TrackerInfo::Enum >const & _info )
{
	readWriteTrackerInfo( _stream,_trackerStats, common::CReadString(), _info );
}

template < class S >
void
writeTrackerInfo( S & _stream ,common::CTrackerStats const & _trackerStats, std::vector< TrackerInfo::Enum >const & _info )
{
	readWriteTrackerInfo( _stream,_trackerStats, common::CReadWrite(), _info );
}

}
#endif //HELPER_H
