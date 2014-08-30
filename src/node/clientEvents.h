// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CLIENTEVENTS_H
#define CLIENTEVENTS_H

namespace client
{

struct CNetworkDiscoveredEvent : boost::statechart::event< CDnsInfo >
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

struct CTransactionAckEvent : boost::statechart::event< CTransactionAckEvent >
{
	CTransactionAckEvent( common::TransactionsStatus::Enum _status, CTransaction _transactionSend ): m_status( _status ), m_transactionSend( _transactionSend ){}
	common::TransactionsStatus::Enum m_status;
	CTransaction m_transactionSend;
};


}

#endif // CLIENTEVENTS_H
