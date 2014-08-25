// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef VALIDATE_TRANSACTION_ACTION_EVENTS_H
#define VALIDATE_TRANSACTION_ACTION_EVENTS_H

#include <boost/statechart/event.hpp>
#include "main.h"

namespace tracker
{

struct CValidationEvent : boost::statechart::event< CValidationEvent >
{
	CValidationEvent( bool _valid ):m_valid( _valid ){};
	bool m_valid;
};

struct CMerkleBlocksEvent : boost::statechart::event< CMerkleBlocksEvent >
{
	CMerkleBlocksEvent( std::vector< CMerkleBlock > const & _merkles, std::map< uint256 ,std::vector< CTransaction > > const & _transactions,long long _id ):m_merkles( _merkles ), m_transactions( _transactions ),m_id( _id ){};
	std::vector< CMerkleBlock > m_merkles;
	std::map< uint256 ,std::vector< CTransaction > > m_transactions;
	long long const m_id;
};

struct CSynchronizationInfoEvent : boost::statechart::event< CSynchronizationInfoEvent >
{
	CSynchronizationInfoEvent( uint64_t _timeStamp, unsigned int _nodeIdentifier ):m_timeStamp( _timeStamp ),m_nodeIdentifier(_nodeIdentifier){}

	uint64_t const m_timeStamp;
	unsigned int m_nodeIdentifier;
};

struct CTransactionBlockEvent : boost::statechart::event< CTransactionBlockEvent >
{
/*	CTransactionBlockEvent( CDiskBlock * _discBlock ):m_discBlock( _discBlock )
	{
	}
	CDiskBlock * m_discBlock;*/
};

struct CSwitchToSynchronizing : boost::statechart::event< CSwitchToSynchronizing >
{
};

struct CSwitchToSynchronized : boost::statechart::event< CSwitchToSynchronized >
{
};

struct CGetNextBlockEvent : boost::statechart::event< CGetNextBlockEvent >
{
};

}


#endif // VALIDATE_TRANSACTION_ACTION_EVENTS_H
