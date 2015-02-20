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
#include "tracker/trackerRequests.h"

namespace tracker
{

unsigned int const LoopTime = 20;

struct CProvideInfo : boost::statechart::state< CProvideInfo, CProvideInfoAction >
{
	CProvideInfo( my_context ctx ) : my_base( ctx )
	{
		m_enterStateTime = GetTime();
	}

	boost::statechart::result react( const common::CContinueEvent & _continueEvent )
	{
		int64_t time = GetTime();
		if ( time - m_enterStateTime < LoopTime )
		{
			context< CProvideInfoAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CProvideInfoAction >().getNodeIndicator() ) ) );
		}
		else
		{
			context< CProvideInfoAction >().setRequest( 0 );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		context< CProvideInfoAction >().setRequest( 0 );
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CInfoRequestData requestedInfo;

		common::convertPayload( orginalMessage, requestedInfo );

		context< CProvideInfoAction >().setRequest( new CDeliverInfoRequest( new CSpecificMediumFilter( context< CProvideInfoAction >().getNodeIndicator() ) ) );
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

CProvideInfoAction::CProvideInfoAction( uint256 const & _actionKey, uintptr_t _nodeIndicator )
	: CCommunicationAction( _actionKey )
	, m_request( 0 )
	, m_nodeIndicator( _nodeIndicator )
{
	initiate();
	process_event( common::CSwitchToConnectedEvent() );
}

common::CRequest< TrackerResponses >*
CProvideInfoAction::execute()
{
	common::CRequest< TrackerResponses >* request = m_request;
	m_request = 0;
	return request;
}

void
CProvideInfoAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CProvideInfoAction::setRequest( common::CRequest< TrackerResponses >* _request )
{
	m_request = _request;
}

common::CRequest< TrackerResponses > const *
CProvideInfoAction::getRequest() const
{
	return m_request;
}

uintptr_t
CProvideInfoAction::getNodeIndicator() const
{
	return m_nodeIndicator;
}

}


