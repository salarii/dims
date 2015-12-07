// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/communicationProtocol.h"
#include "common/requests.h"
#include "common/responses.h"
#include "common/setResponseVisitor.h"

#include "monitor/updateNetworkDataAction.h"
#include "monitor/reputationTracer.h"
#include "monitor/filters.h"

namespace monitor
{

struct CUpdateNetworkData : boost::statechart::state< CUpdateNetworkData, CUpdateNetworkDataAction >
{
	CUpdateNetworkData( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CRankingFullInfo rankingFullInfo;

		common::convertPayload( orginalMessage, rankingFullInfo );

		CReputationTracker::getInstance()->updateRankingInfo( _messageResult.m_pubKey, rankingFullInfo );

		context< CUpdateNetworkDataAction >().addRequest(
		new common::CAckRequest(
						context< CUpdateNetworkDataAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

		context< CUpdateNetworkDataAction >().setExit();

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

	std::set< uint160 > m_presentTrackers;
};

CUpdateNetworkDataAction::CUpdateNetworkDataAction( uint256 const & _actionKey )
	: common::CAction( _actionKey )
{
	LogPrintf("update network action: %p \n", this );

	initiate();
}

void
CUpdateNetworkDataAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}
