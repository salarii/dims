// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "registerAction.h"

#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/commonRequests.h"
#include "common/setResponseVisitor.h"

#include "tracker/trackerRequests.h"

namespace tracker
{

//milisec
unsigned int const WaitTime = 20000;

struct CFreeRegistration;

struct CInitiateRegistration : boost::statechart::state< CInitiateRegistration, CRegisterAction >
{
	CInitiateRegistration( my_context ctx )
		: my_base( ctx )
	{
		LogPrintf("register action: %p initiate registration \n", &context< CRegisterAction >() );
		context< CRegisterAction >().dropRequests();
		context< CRegisterAction >().addRequests(
		 new common::CTimeEventRequest< common::CTrackerTypes >( WaitTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CRegisterAction >().addRequests(
					new CAskForRegistrationRequest(
						context< CRegisterAction >().getActionKey()
						, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CRegistrationTerms connectCondition;

		common::convertPayload( orginalMessage, connectCondition );

		if ( !connectCondition.m_price )
		{
			context< CRegisterAction >().dropRequests();

			context< CRegisterAction >().addRequests(
						new common::CAckRequest< common::CTrackerTypes >(
							  context< CRegisterAction >().getActionKey()
							, connectCondition.m_id
							, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) ) );

			return transit< CFreeRegistration >();
		}
		else
			//do something later
			return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		assert( !"no response" );
		context< CRegisterAction >().dropRequests();
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

struct CFreeRegistration : boost::statechart::state< CFreeRegistration, CRegisterAction >
{
	CFreeRegistration( my_context ctx )
		: my_base( ctx )
	{
		LogPrintf("register action: %p free registration \n", &context< CRegisterAction >() );

		context< CRegisterAction >().addRequests(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						WaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CRegisterAction >().addRequests(
					new CRegisterProofRequest(
						context< CRegisterAction >().getActionKey()
						, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CResult result;

		common::convertPayload( orginalMessage, result );

		assert( result.m_result );// for debug only, do something here

		context< CRegisterAction >().addRequests(
					new common::CAckRequest< common::CTrackerTypes >(
						  context< CRegisterAction >().getActionKey()
						, result.m_id
						, new CSpecificMediumFilter( context< CRegisterAction >().getNodePtr() ) ) );
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CRegisterAction >().dropRequests();
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

CRegisterAction::CRegisterAction( uintptr_t _nodePtr )
	: CCommunicationAction( getActionKey() )
	, m_nodePtr( _nodePtr )
{
	initiate();
}

void
CRegisterAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
