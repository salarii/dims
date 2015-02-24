// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"
#include "common/authenticationProvider.h"
#include "common/mediumRequests.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "monitor/filters.h"
#include "monitor/monitorRequests.h"
#include "monitor/reputationTracer.h"
#include "monitor/monitorController.h"
#include "monitor/updateDataAction.h"

namespace monitor
{

unsigned int const LoopTime = 20;

struct CAskForUpdate : boost::statechart::state< CAskForUpdate, CUpdateDataAction >
{
	CAskForUpdate( my_context ctx ) : my_base( ctx )
	{
		m_enterStateTime = GetTime();
		context< CUpdateDataAction >().clearRequests();
		context< CUpdateDataAction >().addRequests( new CInfoRequest( context< CUpdateDataAction >().getActionKey(), new CMediumClassFilter( common::CMediumKinds::Trackers ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _result )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _result.m_message, orginalMessage, GetTime(), _result.m_pubKey ) )
			assert( !"service it somehow" );

		common::CKnownNetworkInfo knownNetworkInfo;

		common::convertPayload( orginalMessage, knownNetworkInfo );// right  now it is not clear to me what to  do with  this

		std::vector< common::CValidNodeInfo > validNodesInfo;
		context< CUpdateDataAction >().clearRequests();
		context< CUpdateDataAction >().addRequests( new common::CAckRequest< MonitorResponses >( context< CUpdateDataAction >().getActionKey(), new CSpecificMediumFilter( _result.m_nodeIndicator ) ) );

		m_presentTrackers.insert( _result.m_pubKey.GetID() );

		return discard_event();
	}

	boost::statechart::result react( common::CAckPromptResult const & _ackPrompt )
	{
		context< CUpdateDataAction >().clearRequests();
		return discard_event();
	}

	boost::statechart::result react( common::CNoMedium const & _ackPrompt )
	{
		context< CUpdateDataAction >().clearRequests();
		return discard_event();
	}

	int64_t m_enterStateTime;

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CNoMedium >,
	boost::statechart::custom_reaction< common::CAckPromptResult >
	> reactions;

	std::set< uint160 > m_presentTrackers;
};

CUpdateDataAction::CUpdateDataAction( bool _autoDelete )
: common::CAction< MonitorResponses >( _autoDelete )
{
	initiate();
	process_event( common::CSwitchToConnectedEvent() );
}

void
CUpdateDataAction::accept( common::CSetResponseVisitor< MonitorResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CUpdateDataAction::reset()
{
	common::CAction< MonitorResponses >::reset();
	initiate();
}

}

