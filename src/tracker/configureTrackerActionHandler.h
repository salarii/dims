// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONFIGURE_TRACKER_ACTION_HANDLER_H
#define CONFIGURE_TRACKER_ACTION_HANDLER_H

#include <boost/mpl/list.hpp>
#include <boost/variant/variant.hpp>

#include "common/commonResponses.h"
#include "common/commonEvents.h"

#include "main.h"

namespace common
{
struct CDiskBlock;

class CSegmentHeader;
}

namespace tracker
{
struct CDummyResponse{};

struct CValidationResult
{
	CValidationResult( std::vector< unsigned int > const & _invalidTransactionIndexes ):m_invalidTransactionIndexes( _invalidTransactionIndexes ){};
	std::vector< unsigned int > m_invalidTransactionIndexes;
};

struct CSynchronizationInfoResult : boost::statechart::event< CSynchronizationInfoResult >
{
	CSynchronizationInfoResult( uint64_t const _timeStamp, uintptr_t _nodeIndicator, uint256 const & m_id ):m_timeStamp( _timeStamp ),m_nodeIndicator( _nodeIndicator ){};

	uint64_t m_timeStamp;
	uintptr_t m_nodeIndicator;
	uint256 m_id;
};

template < class Block >
struct CSynchronizationBlockResult
{
	CSynchronizationBlockResult(){}

	CSynchronizationBlockResult( Block * _discBlock, unsigned int _blockIndex ):m_discBlock( _discBlock ), m_blockIndex( _blockIndex ){}

	Block * m_discBlock;

	unsigned int m_blockIndex;
};

// list all desired types
typedef boost::mpl::list< common::ScheduledResult, common::CPingPongResult, common::CMediumException, CDummyResponse, common::CAvailableCoins, CValidationResult, common::CConnectedNode, common::CIdentificationResult, common::CRequestedMerkles, CSynchronizationInfoResult, common::CAckResult, CSynchronizationBlockResult< common::CSegmentHeader >, CSynchronizationBlockResult< common::CDiskBlock >, common::CEndEvent, common::CMessageResult, common::CTimeEvent > TrackerResponseList;

typedef boost::make_variant_over< TrackerResponseList >::type TrackerResponses;
}

#endif // CONFIGURE_TRACKER_ACTION_HANDLER_H
