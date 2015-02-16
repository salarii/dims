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
#include "monitor/monitorNodesManager.h"
#include "monitor/reputationTracer.h"
#include "monitor/monitorController.h"
#include "monitor/monitorNodeMedium.h"
#include "monitor/updateDataAction.h"

namespace monitor
{

unsigned int const LoopTime = 20;

struct CAskForUpdate : boost::statechart::state< CAskForUpdate, CUpdateDataAction >
{
	CAskForUpdate( my_context ctx ) : my_base( ctx )
	{
		m_enterStateTime = GetTime();
		context< CUpdateDataAction >().setRequest( new CInfoRequest( new CMediumClassFilter( common::RequestKind::Trackers ) ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		int64_t time = GetTime();
		if ( time - m_enterStateTime < LoopTime )
		{
			context< CUpdateDataAction >().setRequest( new common::CContinueReqest<MonitorResponses>( _continueEvent.m_keyId, new CMediumClassFilter( common::RequestKind::Trackers ) ) );
		}
		else
		{
			//here  make  some  data manipulation
			context< CUpdateDataAction >().setRequest( 0 );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _result )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _result.m_message, orginalMessage, GetTime(), _result.m_pubKey ) )
			assert( !"service it somehow" );

		common::CKnownNetworkInfo knownNetworkInfo;

		// save  this  stuff
		common::convertPayload( orginalMessage, knownNetworkInfo );// right  now it is not clear to me what to  do with  this

		std::vector< common::CValidNodeInfo > validNodesInfo;
		context< CUpdateDataAction >().setRequest( new common::CAckRequest< MonitorResponses >( context< CUpdateDataAction >().getActionKey(), new CSpecificMediumFilter( _result.m_nodeIndicator ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CAckPromptResult const & _ackPrompt )
	{
		context< CUpdateDataAction >().setRequest( new common::CContinueReqest<MonitorResponses>( context< CUpdateDataAction >().getActionKey(), new CMediumClassFilter( common::RequestKind::Trackers ) ) );
	}

	int64_t m_enterStateTime;

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
		boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckPromptResult >
	> reactions;
};

CUpdateDataAction::CUpdateDataAction()
: m_request( 0 )
{
	initiate();
	process_event( common::CSwitchToConnectedEvent() );
}

common::CRequest< MonitorResponses >*
CUpdateDataAction::execute()
{
	common::CRequest< MonitorResponses >* request = m_request;
	m_request = 0;
	return request;
}

void
CUpdateDataAction::accept( common::CSetResponseVisitor< MonitorResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CUpdateDataAction::setRequest( common::CRequest< MonitorResponses >* _request )
{
	m_request = _request;
}

common::CRequest< MonitorResponses > const *
CUpdateDataAction::getRequest() const
{
	return m_request;
}

}

