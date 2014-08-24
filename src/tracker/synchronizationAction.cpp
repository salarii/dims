// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "common/setResponseVisitor.h"
#include "common/mediumRequests.h"
#include "common/commonEvents.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/mediumKinds.h"

#include "synchronizationAction.h"
#include "synchronizationRequests.h"
#include "trackerFilters.h"
#include "synchronizationEvents.h"
#include "segmentFileStorage.h"

#define CONFIRM_LIMIT 6

namespace tracker
{

unsigned const SynchronisingGetInfoTime = 10;

unsigned const SynchronisingWaitTime = 15;

unsigned const SynchronisedWaitTime = SynchronisingWaitTime * 2;

struct CSynchronizingGetInfo;
struct CSynchronizedGetInfo;



struct CSynchronizationInfoEvent : boost::statechart::event< CSynchronizationInfoEvent >
{
	CSynchronizationInfoEvent( uint64_t _timeStamp, unsigned int _nodeIdentifier ):m_timeStamp( _timeStamp ),m_nodeIdentifier(_nodeIdentifier){}

	uint64_t const m_timeStamp;
	unsigned int m_nodeIdentifier;
};

struct CDiskBlock;

struct CTransactionBlockEvent : boost::statechart::event< CTransactionBlockEvent >
{
	CTransactionBlockEvent( CDiskBlock * _discBlock ):m_discBlock( _discBlock )
	{
	}
	CDiskBlock * m_discBlock;
};

struct CSynchronizeRequestEvent : boost::statechart::event< CTransactionBlockEvent >
{
};

struct CUninitiated : boost::statechart::simple_state< CUninitiated, CSynchronizationAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CSwitchToSynchronizing, CSynchronizingGetInfo >,
	boost::statechart::transition< CSwitchToSynchronized, CSynchronizedGetInfo >
	> reactions;
};

struct CSynchronizing;

struct CSynchronizingGetInfo : boost::statechart::state< CSynchronizingGetInfo, CSynchronizationAction >
{
	CSynchronizingGetInfo( my_context ctx ) : my_base( ctx ), m_waitTime( SynchronisingWaitTime )
	{
		context< CSynchronizationAction >().setRequest( new CGetSynchronizationInfoRequest( context< CSynchronizationAction >().getActionKey(), 0 ) );
		m_waitTime = GetTime();
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		long long unsigned time = GetTime();
		if ( time - m_waitTime < SynchronisingGetInfoTime )
			context< CSynchronizationAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CMediumClassFilter( common::CMediumKinds::Trackers ) ) );
		else
			return transit< CSynchronizing >();
	}

	boost::statechart::result react( CSynchronizationInfoEvent const & _synchronizationInfoEvent )
	{
		if ( m_bestTimeStamp < _synchronizationInfoEvent.m_timeStamp )
		{
			m_bestTimeStamp = _synchronizationInfoEvent.m_timeStamp;
			context< CSynchronizationAction >().setNodeIdentifier( _synchronizationInfoEvent.m_nodeIdentifier );
		}
		context< CSynchronizationAction >().setRequest( new common::CContinueReqest<TrackerResponses>( context< CSynchronizationAction >().getActionKey(), new CMediumClassFilter( common::CMediumKinds::Trackers ) ) );
	}


	typedef boost::mpl::list<
	boost::statechart::custom_reaction< CSynchronizationInfoEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;

	long long unsigned m_waitTime;

	// best synchronising option
	uint64_t m_bestTimeStamp;
};

struct CSynchronized;

struct CSynchronizedGetInfo : boost::statechart::state< CSynchronizedGetInfo, CSynchronizationAction >
{
	CSynchronizedGetInfo( my_context ctx ) : my_base( ctx ), m_waitTime( SynchronisedWaitTime )
	{
		context< CSynchronizationAction >().setRequest( new CGetSynchronizationInfoRequest(
															  context< CSynchronizationAction >().getActionKey()
															, CSegmentFileStorage::getInstance()->getTimeStampOfLastFlush()
															, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) )
														);
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		if ( m_waitTime-- )
			context< CSynchronizationAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CMediumClassFilter( common::CMediumKinds::DimsNodes ) ) );
//		else
//			return transit< CSynchronizing >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::transition< CSynchronizeRequestEvent, CSynchronized >
	> reactions;

	unsigned int m_waitTime;

	// best synchronising option
	uint64_t m_bestTimeStamp;
};

struct CSynchronizing : boost::statechart::state< CSynchronizing, CSynchronizationAction >
{
	CSynchronizing( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().setRequest(
					new CSynchronizationAssistanceRequest( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
	}

	boost::statechart::result react( common::CAckEvent const & _ackEvent )
	{
		context< CSynchronizationAction >().setRequest(
					new CGetNextBlockRequest( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
	}

	boost::statechart::result react( CTransactionBlockEvent const & _transactionBlockEvent )
	{
		//_transactionBlockEvent.m_discBlock  work in  progress

		if ( _transactionBlockEvent.m_discBlock )
			context< CSynchronizationAction >().setRequest(
						new CGetNextBlockRequest( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
		else
			context< CSynchronizationAction >().setRequest( 0 );
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
			context< CSynchronizationAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< CTransactionBlockEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;
};

struct CSynchronized : boost::statechart::state< CSynchronized, CSynchronizationAction >
{
	CSynchronized( my_context ctx ) : my_base( ctx ),m_currentBlock( 0 )
	{
		m_storedBlocks = CSegmentFileStorage::getInstance()->calculateStoredBlockNumber();
	}

	boost::statechart::result react( CGetNextBlockEvent const & _getNextBlockEvent )
	{
		//_transactionBlockEvent.m_discBlock  work in  progress
		if ( m_currentBlock < m_storedBlocks )
		{
			CDiskBlock * diskBlock = new CDiskBlock;
			CSegmentFileStorage::getInstance()->getBlock( m_currentBlock, *diskBlock );


		}
		else
			;//end request
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
			context< CSynchronizationAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< CGetNextBlockEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;

	unsigned int m_storedBlocks;

	unsigned int m_currentBlock;
};

CSynchronizationAction::CSynchronizationAction()
	: m_request( 0 )
{
	initiate();
}

CSynchronizationAction::CSynchronizationAction( uint256 const & _actionKey, uintptr_t _nodeIndicator, uint64_t _timeStamp )
	: common::CCommunicationAction( _actionKey )
	, m_request( 0 )
	, m_nodeIdentifier( _nodeIndicator )
	, m_timeStamp( _timeStamp )
{
	initiate();
	process_event( CSwitchToSynchronized() );
}

common::CRequest< TrackerResponses >*
CSynchronizationAction::execute()
{
	common::CRequest< TrackerResponses > * request = m_request;
	m_request = 0;
	return request;
}

void
CSynchronizationAction::accept( common::CSetResponseVisitor< TrackerResponses > & _visitor )
{
	_visitor.visit( *this );
}

void
CSynchronizationAction::setRequest( common::CRequest< TrackerResponses >* _request )
{
	m_request = _request;
}

void
CSynchronizationAction::clear()
{

}

unsigned long long
CSynchronizationAction::getNodeIdentifier() const
{
	return m_nodeIdentifier;
}

bool
CSynchronizationAction::isRequestInitialized() const
{
	return m_request;
}

void
CSynchronizationAction::setNodeIdentifier( unsigned int _nodeIdentifier )
{
	m_nodeIdentifier = _nodeIdentifier;
}

}

//struct CDiskBlock;
