
// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/commonEvents.h"
#include "common/authenticationProvider.h"
#include "common/commonRequests.h"
#include "common/communicationProtocol.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "monitor/provideInfoAction.h"
#include "monitor/monitorRequests.h"
#include "monitor/filters.h"

namespace monitor
{

unsigned int const LoopTime = 20000;//milisec

struct CIsRegisteredInfo;

struct CProvideInfo : boost::statechart::state< CProvideInfo, CProvideInfoAction >
{
	CProvideInfo( my_context ctx ) : my_base( ctx )
	{
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CInfoRequestData requestedInfo;

		common::convertPayload( orginalMessage, requestedInfo );

		context< CProvideInfoAction >().setInfoRequestKey( _messageResult.m_message.m_header.m_id );

		context< CProvideInfoAction >().dropRequests();

		context< CProvideInfoAction >().addRequest(
					new common::CAckRequest< common::CMonitorTypes >(
						  context< CProvideInfoAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CProvideInfoAction >().getNodeIndicator() ) ) );

		if ( ( common::CInfoKind::Enum )requestedInfo.m_kind == common::CInfoKind::IsRegistered )
		{
			return transit< CIsRegisteredInfo >();
		}

//		context< CProvideInfoAction >().addRequest( new CDeliverInfoRequest( context< CProvideInfoAction >().getActionKey(), new CSpecificMediumFilter( context< CProvideInfoAction >().getNodeIndicator() ) ) );

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

struct CIsRegisteredInfo : boost::statechart::state< CIsRegisteredInfo, CProvideInfoAction >
{
	CIsRegisteredInfo( my_context ctx ) : my_base( ctx )
	{
		context< CProvideInfoAction >().addRequest(
					new common::CTimeEventRequest< common::CMonitorTypes >(
						LoopTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		context< CProvideInfoAction >().dropRequests();
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CProvideInfoAction >().dropRequests();
		context< CProvideInfoAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CMonitorStop : boost::statechart::state< CMonitorStop, CProvideInfoAction >
{
	CMonitorStop( my_context ctx ) : my_base( ctx )
	{
	}
};

CProvideInfoAction::CProvideInfoAction( uint256 const & _actionKey, uintptr_t _nodeIndicator )
	: common::CScheduleAbleAction< common::CMonitorTypes >( _actionKey )
	, m_registerObject( _actionKey )
	, m_nodeIndicator( _nodeIndicator )
{
	initiate();
	process_event( common::CSwitchToConnectedEvent() );
}

void
CProvideInfoAction::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
{
	_visitor.visit( *this );
}

uintptr_t
CProvideInfoAction::getNodeIndicator() const
{
	return m_nodeIndicator;
}

}


