// Copyright (c) 2014-2015 Dims dev-team
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
	CValidationEvent( std::vector< unsigned int > const & _invalidTransactionIndexes ):m_invalidTransactionIndexes( _invalidTransactionIndexes ){};
	std::vector< unsigned int > m_invalidTransactionIndexes;
};

struct CSynchronizationInfoEvent : boost::statechart::event< CSynchronizationInfoEvent >
{
	CSynchronizationInfoEvent( uint64_t _timeStamp, unsigned int _nodeIdentifier ):m_timeStamp( _timeStamp ),m_nodeIdentifier(_nodeIdentifier){}

	uint64_t const m_timeStamp;
	unsigned int m_nodeIdentifier;
};

template < class Block >
struct CTransactionBlockEvent : boost::statechart::event< CTransactionBlockEvent<Block> >
{
	CTransactionBlockEvent( Block * _discBlock, unsigned int _blockIndex ):m_discBlock( _discBlock ),m_blockIndex( _blockIndex )
	{
	}
	Block * m_discBlock;
	unsigned int m_blockIndex;
};

struct CSwitchToSynchronizing : boost::statechart::event< CSwitchToSynchronizing >
{
};

struct CSwitchToSynchronized : boost::statechart::event< CSwitchToSynchronized >
{
};


}


#endif // VALIDATE_TRANSACTION_ACTION_EVENTS_H
