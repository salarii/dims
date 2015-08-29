
// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "core.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/authenticationProvider.h"
#include "common/setResponseVisitor.h"
#include "common/analyseTransaction.h"
#include "common/commonRequests.h"

#include "monitor/enterNetworkAction.h"
#include "monitor/monitorController.h"
#include "monitor/admitTransactionsBundle.h"
#include "monitor/filters.h"
#include "monitor/reputationTracer.h"
#include "monitor/monitorRequests.h"
#include "monitor/rankingDatabase.h"

namespace monitor
{

//milisec
unsigned int const WaitTime = 10000;


struct CEnterNetworkInitial : boost::statechart::simple_state< CEnterNetworkInitial, CEnterNetworkAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< ,  >,
	boost::statechart::transition< ,  >
	> reactions;
};

struct CAskForAddmision : boost::statechart::state< CAskForAddmision, CEnterNetworkAction >
{
	CAskForAddmision( my_context ctx ): my_base( ctx )
	{
		context< CEnterNetworkAction >().addRequest(
					new common::CSendMessageRequest< common::CMonitorTypes >(
						common::CPayloadKind::EnterNetworkAsk
						, context< CEnterNetworkAction >().getActionKey()
						, new CMediumClassFilter( common::CMediumKinds::Monitors, 1 ) ) );


		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						 WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}
	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{

	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		// not  responding  do  something
		context< CEnterNetworkAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CAckEvent, CSynchronization >,
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;
};


struct CSynchronization : boost::statechart::state< CSynchronization, CEnterNetworkAction >
{
	CSynchronization( my_context ctx )
		: my_base( ctx )
	{
		context< CEnterNetworkAction >().dropRequests();
		context< CEnterNetworkAction >().addRequest(
		 new common::CTimeEventRequest< common::CMonitorTypes >(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CEnterNetworkAction >().addRequest(
					new common::CScheduleActionRequest< common::CMonitorTypes >(
						new CSynchronizationAction( context< CEnterNetworkAction >().getNodePtr() )
						, new CMediumClassFilter( common::CMediumKinds::Schedule) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CEnterNetworkAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CSynchronizationResult const & _synchronizationResult )
	{
		if ( _synchronizationResult.m_result )
		{
			return transit< CFetchRankingTimeAndInfo >();
		}

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CSynchronizationResult >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CFetchRankingTimeAndInfo : boost::statechart::state< CFetchRankingTimeAndInfo, CEnterNetworkAction >
{
	CFetchRankingTimeAndInfo( my_context ctx ): my_base( ctx )
	{
		common::CSendMessageRequest< common::CMonitorTypes > * request =
				new common::CSendMessageRequest< common::CMonitorTypes >(
					common::CPayloadKind::InfoReq
					, context< CEnterNetworkAction >().getActionKey()
					, new CSpecificMediumFilter( context< CEnterNetworkAction >().getNodePtr() ) )

				request->addPayload( (int)common::CInfoKind::StorageInfoAsk, std::vector<unsigned char>() )

				context< CEnterNetworkAction >().addRequest( request );

		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
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

	boost::statechart::result react( common::CTimeEvent const & _timeEvent ){}

	boost::statechart::result react( common::CAckEvent const & _ackEvent ){}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

struct CSendRankingTimeAndInfo : boost::statechart::state< CSendRankingTimeAndInfo, CEnterNetworkAction >
{
	CSendRankingTimeAndInfo( my_context ctx ): my_base( ctx ){}
	boost::statechart::result react( common::CMessageResult const & _messageResult ){}
	boost::statechart::result react( common::CTimeEvent const & _timeEvent ){}
	boost::statechart::result react( common::CAckEvent const & _ackEvent ){}
	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

struct  : boost::statechart::state< , CEnterNetworkAction >
{
	( my_context ctx )
		: my_base( ctx )
	{
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CAdmitProof admitMessage;

		common::convertPayload( orginalMessage, admitMessage );

		CReputationTracker::getInstance()->addTracker( CTrackerData( _messageResult.m_pubKey, 0, CMonitorController::getInstance()->getPeriod(), GetTime() ) );

		context< CEnterNetworkAction >().addRequest(
					new common::CAckRequest< common::CMonitorTypes >(
						context< CEnterNetworkAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( context< CEnterNetworkAction >().getNodePtr() ) ) );

		context< CEnterNetworkAction >().addRequest(
					new common::CResultRequest< common::CMonitorTypes >(
						  context< CEnterNetworkAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, 1
						, new CSpecificMediumFilter( context< CEnterNetworkAction >().getNodePtr() ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

};


CEnterNetworkAction::CEnterNetworkAction( uint256 const & _actionKey, uintptr_t _nodePtr )
	: common::CAction< common::CMonitorTypes >( _actionKey )
	, m_nodePtr( _nodePtr )
{
	initiate();
}

void
CEnterNetworkAction::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
