// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "connectNodeAction.h"
#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"
#include "common/authenticationProvider.h"
#include "common/mediumRequests.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "tracker/provideInfoAction.h"

namespace tracker
{

unsigned int const LoopTime = 20;

struct CProvideInfo : boost::statechart::state< CProvideInfo, CProvideInfoAction >
{
	CProvideInfo( my_context ctx ) : my_base( ctx )
	{
		m_enterStateTime = GetTime();
		context< CProvideInfoAction >().setRequest( new CInfoRequest( new CMediumClassFilter( common::RequestKind::Trackers ) ) );
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		int64_t time = GetTime();
		if ( time - m_enterStateTime < SeedLoopTime )
		{
			context< CAskForUpdate >().setRequest( new common::CContinueReqest<MonitorResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CProvideInfoAction >().getMediumPtr() ) ) );
		}
		else
		{
			context< CAskForUpdate >().setRequest( 0 );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _result )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _result.m_message, orginalMessage, GetTime(), _result.m_message ) )
			assert( !"service it somehow" );

		common::CKnownNetworkInfo knownNetworkInfo;

		// save  this  stuff

		common::convertPayload( orginalMessage, knownNetworkInfo );// right  now it is not clear to me what to  do with  this

		std::vector< common::CValidNodeInfo > validNodesInfo;

		context< CProvideInfoAction >().setRequest( new common::CKnownNetworkInfoRequest< MonitorResponses >( context< CProvideInfoAction >().getActionKey(), validNodesInfo, new CSpecificMediumFilter( context< CProvideInfoAction >().getMediumPtr() ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return transit< CMonitorStop >();
	}

	int64_t m_enterStateTime;

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
		boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CMonitorStop : boost::statechart::state< CMonitorStop, CProvideInfoAction >
{
	CMonitorStop( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		context< CProvideInfoAction >().setRequest( 0 );
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};

CProvideInfoAction::CProvideInfoAction( uint256 const & _actionKey )
	: CCommunicationAction( _actionKey )
	, m_request( 0 )
{
	initiate();
	process_event( common::CSwitchToConnectedEvent() );
}

common::CRequest< MonitorResponses >*
CProvideInfoAction::execute()
{
	common::CRequest< MonitorResponses >* request = m_request;
	m_request = 0;
	return request;
}

void
CProvideInfoAction::accept( common::CSetResponseVisitor< MonitorResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CProvideInfoAction::setRequest( common::CRequest< MonitorResponses >* _request )
{
	m_request = _request;
}

common::CRequest< MonitorResponses > const *
CProvideInfoAction::getRequest() const
{
	return m_request;
}

}


