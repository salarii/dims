// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/requests.h"
#include "common/events.h"
#include "common/setResponseVisitor.h"

#include "monitor/reputationControlAction.h"
#include "monitor/reputationTracer.h"
#include "monitor/filters.h"

namespace monitor
{
CReputationControlAction * CReputationControlAction::ms_instance = NULL;


struct COperating;
namespace
{

int64_t RecalculationTime;
int64_t calculateNextTime()
{
	RecalculationTime = CReputationTracker::getInstance()->getMeasureReputationTime();
	int64_t period = CReputationTracker::getInstance()->getRecalculateTime();
	int64_t nextTime = RecalculationTime + period;
	nextTime -= GetTime();
	assert( nextTime > 0 );// this  failing means, repotation controller wrongly created
	return nextTime;
}
}

struct CReputationControlInitial : boost::statechart::state< CReputationControlInitial, CReputationControlAction >
{
	CReputationControlInitial( my_context ctx ) : my_base( ctx )
	{
		context< CReputationControlAction >().forgetRequests();
		context< CReputationControlAction >().addRequest(
					new common::CTimeEventRequest(
						calculateNextTime()
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		m_allyMonitorData = CReputationTracker::getInstance()->getAllyMonitors();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		CReputationTracker::getInstance()->clearTransactions();
		m_allyMonitorData = CReputationTracker::getInstance()->getAllyMonitors();
		RecalculationTime += CReputationTracker::getInstance()->getRecalculateTime();

		context< CReputationControlAction >().addRequest(
					new common::CTimeEventRequest(
						calculateNextTime()
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( _messageResult.m_message.m_header.m_payloadKind == common::CPayloadKind::RankingInfo )
		{
			common::CRankingInfo rankingInfo;
			common::convertPayload( orginalMessage, rankingInfo );

			context< CReputationControlAction >().addRequest(
						new common::CAckRequest(
							context< CReputationControlAction >().getActionKey()
							, orginalMessage.m_header.m_id
							, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

			// do  some  sanity??
			m_allyMonitorData.erase( _messageResult.m_pubKey );
			BOOST_FOREACH( common::CTrackerData const & trackerData, rankingInfo.m_trackers )
			{
				CReputationTracker::getInstance()->addAllyTracker( common::CAllyTrackerData( trackerData, _messageResult.m_pubKey ) );
			}
			if ( m_allyMonitorData.empty() )
				return transit< COperating >();//ready to start normal operations
		}
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	std::set< common::CAllyMonitorData > m_allyMonitorData;

};

struct COperating : boost::statechart::state< COperating, CReputationControlAction >
{
	COperating( my_context ctx ) : my_base( ctx )
	{
		context< CReputationControlAction >().forgetRequests();
		context< CReputationControlAction >().addRequest(
					new common::CTimeEventRequest(
						calculateNextTime()
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		RecalculationTime += CReputationTracker::getInstance()->getRecalculateTime();
		CReputationTracker::getInstance()->recalculateReputation();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::RankingInfo
					, context< CReputationControlAction >().getActionKey()
					, new CMediumClassFilter( common::CMediumKinds::Monitors ) );

		request->addPayload(
					common::CRankingInfo(
						CReputationTracker::getInstance()->getTrackers()
						, RecalculationTime ) );

		context< CReputationControlAction >().addRequest( request );

		return discard_event();
	}


	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( _messageResult.m_message.m_header.m_payloadKind == common::CPayloadKind::RankingInfo )
		{
		common::CRankingInfo rankingInfo;
		common::convertPayload( orginalMessage, rankingInfo );

		context< CReputationControlAction >().addRequest(
					new common::CAckRequest(
						  context< CReputationControlAction >().getActionKey()
						, orginalMessage.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};
/*
send  request


*/


CReputationControlAction *
CReputationControlAction::getInstance()
{
	return ms_instance;
}

CReputationControlAction *
CReputationControlAction::createInstance( uint256 const & _actionKey )
{
	if ( ms_instance )
		ms_instance->setExit();

	ms_instance = new CReputationControlAction( _actionKey );

	return ms_instance;
}

CReputationControlAction *
CReputationControlAction::createInstance()
{
	if ( ms_instance )
		ms_instance->setExit();

	ms_instance = new CReputationControlAction();

	return ms_instance;
}

CReputationControlAction::CReputationControlAction()
{
}

CReputationControlAction::CReputationControlAction( uint256 const & _actionKey )
	: common::CAction( _actionKey )
{
}

void
CReputationControlAction::accept( common::CSetResponseVisitor & _visitor )
{
	_visitor.visit( *this );
}

}
