// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONFIGURE_TRACKER_ACTION_HANDLER_H
#define CONFIGURE_TRACKER_ACTION_HANDLER_H

#include <boost/mpl/list.hpp>
#include <boost/variant/variant.hpp>

#include "common/commonResponses.h"

#include "common/selfNode.h"

#include "main.h"

namespace tracker
{
struct CDummyResponse{};

struct CValidationResult
{
	CValidationResult( bool _valid ):m_valid( _valid ){};
	bool m_valid;
};

struct CRequestedMerkles
{
	CRequestedMerkles( std::vector< CMerkleBlock > const & _merkles, std::vector< CTransaction > const & _transactions, long long _id ):m_merkles( _merkles ),m_transactions( _transactions ), m_id( _id ){};

	std::vector< CMerkleBlock > m_merkles;
	std::vector< CTransaction > m_transactions;
	long long m_id;
};

// list all desired types
typedef boost::mpl::list< common::CMediumException, CDummyResponse, common::CAvailableCoins, CValidationResult, common::CConnectedNode, common::CIdentificationResult, common::CContinueResult, CRequestedMerkles > TrackerResponseList;

typedef boost::make_variant_over< TrackerResponseList >::type TrackerResponses;
}

#endif // CONFIGURE_TRACKER_ACTION_HANDLER_H
