// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLIENTEVENTS_H
#define CLIENTEVENTS_H

#include "common/responses.h"

namespace client
{

struct CNetworkDiscoveredEvent : boost::statechart::event< common::CDnsInfo >
{
	CNetworkDiscoveredEvent( unsigned int _trackers, unsigned int _monitors ): m_trackers( _trackers ), m_monitors( _monitors ){}

	unsigned int m_trackers;
	unsigned int m_monitors;

};

struct CCoinsEvent : boost::statechart::event< CCoinsEvent >
{
	CCoinsEvent( std::map< uint256, CCoins > const & _coins ):m_coins( _coins ){}

	std::map< uint256, CCoins > m_coins;
};

}

#endif // CLIENTEVENTS_H
