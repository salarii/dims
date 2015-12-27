// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef UPDATE_NETWORK_DATA_ACTION_H
#define UPDATE_NETWORK_DATA_ACTION_H

#include "common/action.h"
#include "common/filters.h"
#include "common/mediumKinds.h"
#include "common/communicationProtocol.h"

#include <boost/statechart/state_machine.hpp>

#include "protocol.h"

namespace monitor
{

struct CUpdateDataInit;

class CUpdateNetworkDataAction : public common::CAction, public  boost::statechart::state_machine< CUpdateNetworkDataAction, CUpdateDataInit >
{
public:
	CUpdateNetworkDataAction( common::CRankingFullInfo const & _rankingFullInfo, common::CMediumKinds::Enum _mediumKind );

	CUpdateNetworkDataAction( uint256 const & _actionKey );

	virtual void accept( common::CSetResponseVisitor & _visitor );
public:
	common::CRankingFullInfo m_rankingFullInfo;

	common::CMediumKinds::Enum m_mediumKind;
};


}

#endif // UPDATE_NETWORK_DATA_ACTION_H
