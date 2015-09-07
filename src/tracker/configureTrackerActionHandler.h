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

struct CValidationResult
{
	CValidationResult( std::vector< unsigned int > const & _invalidTransactionIndexes ):m_invalidTransactionIndexes( _invalidTransactionIndexes ){};
	std::vector< unsigned int > m_invalidTransactionIndexes;
};

// list all desired types
typedef boost::mpl::list<
common::ScheduledResult
, common::CMediumException
, common::CAvailableCoinsEvent
, CValidationResult
, common::CConnectedNode
, common::CIdentificationResult
, common::CRequestedMerkles
, common::CMessageResult
, common::CPingPongResult
, common::CAckResult
, common::CTimeEvent > TrackerResponseList;

typedef boost::make_variant_over< TrackerResponseList >::type TrackerResponses;
}

#endif // CONFIGURE_TRACKER_ACTION_HANDLER_H
