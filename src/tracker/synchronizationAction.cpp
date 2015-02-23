// Copyright (c) 2014-2015 Dims dev-team
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

#include "tracker/transactionRecordManager.h"
#include "tracker/synchronizationAction.h"
#include "tracker/trackerRequests.h"
#include "tracker/trackerFilters.h"
#include "tracker/segmentFileStorage.h"
#include "tracker/trackerEvents.h"
#include "tracker/trackerController.h"
#include "tracker/trackerControllerEvents.h"
#include "tracker/supportTransactionsDatabase.h"

namespace tracker
{

struct CBlockKind
{
	enum Enum
	{
		Segment
		, Header
	};
};

unsigned const SynchronisingGetInfoTime = 10;

unsigned const SynchronisingWaitTime = 15;

struct CSynchronizingGetInfo;
struct CSynchronizedGetInfo;

struct CDiskBlock;


struct CUninitiated : boost::statechart::simple_state< CUninitiated, CSynchronizationAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CSwitchToSynchronizing, CSynchronizingGetInfo >,
	boost::statechart::transition< CSwitchToSynchronized, CSynchronizedGetInfo >
	> reactions;
};

struct CSynchronizingHeaders;

struct CSynchronizingGetInfo : boost::statechart::state< CSynchronizingGetInfo, CSynchronizationAction >
{
	CSynchronizingGetInfo( my_context ctx ) : my_base( ctx ), m_waitTime( SynchronisingWaitTime )
	{
		context< CSynchronizationAction >().setRequest( new CGetSynchronizationInfoRequest( context< CSynchronizationAction >().getActionKey(), 0 ) );
		m_waitTime = GetTime();
		m_bestTimeStamp = 0;
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		long long unsigned time = GetTime();
		if ( time - m_waitTime < SynchronisingGetInfoTime )
			context< CSynchronizationAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CMediumClassFilter( common::CMediumKinds::Trackers ) ) );
		else if ( m_bestTimeStamp > 0 )
		{
			context< CSynchronizationAction >().setRequest( 0 );
			return transit< CSynchronizingHeaders >();
		}
		else
			return discard_event();
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
	CSynchronizedGetInfo( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().setRequest( new CGetSynchronizationInfoRequest(
															context< CSynchronizationAction >().getActionKey()
															, CSegmentFileStorage::getInstance()->getTimeStampOfLastFlush()
															, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) )
														);

		m_waitTime = GetTime();
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{

		if ( GetTime() - m_waitTime < SynchronisingWaitTime * 2 )
			context< CSynchronizationAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
		else
			context< CSynchronizationAction >().setRequest( 0 );

		return discard_event();
	}

	boost::statechart::result react( common::CGetEvent const & _getEvent )
	{
		return transit< CSynchronized >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CGetEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >
	> reactions;

	unsigned int m_waitTime;

	// best synchronising option
	uint64_t m_bestTimeStamp;
};

struct CSynchronizingBlocks : boost::statechart::state< CSynchronizingBlocks, CSynchronizationAction >
{
	CSynchronizingBlocks( my_context ctx ) : my_base( ctx )
	{
		CTransactionRecordManager::getInstance()->clearCoinViewDB();
		CTransactionRecordManager::getInstance()->clearAddressToCoinsDatabase();
		CTransactionRecordManager::getInstance()->clearSupportTransactionsDatabase();

		context< CSynchronizationAction >().setRequest(
					new CGetNextBlockRequest( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ), (int)CBlockKind::Segment ) );
	}

	boost::statechart::result react( CTransactionBlockEvent< CDiskBlock > const & _transactionBlockEvent )
	{
		std::vector< CTransaction > transactions;

		CSegmentFileStorage::getInstance()->setDiscBlock( *_transactionBlockEvent.m_discBlock, _transactionBlockEvent.m_blockIndex, transactions );

		context< CSynchronizationAction >().setRequest(
					new CGetNextBlockRequest( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ), (int)CBlockKind::Segment ) );

		BOOST_FOREACH( CTransaction const & transaction, transactions )
		{
			CSupportTransactionsDatabase::getInstance()->setTransactionLocation( transaction.GetHash(), transaction.m_location );
		}

		CSupportTransactionsDatabase::getInstance()->flush();

		CTransactionRecordManager::getInstance()->addValidatedTransactionBundle( transactions );

		return discard_event();
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CSynchronizationAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CEndEvent const & )
	{
		context< CSynchronizationAction >().setRequest( new common::CAckRequest< TrackerResponses >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

		return discard_event();
	}


	boost::statechart::result react( common::CAckPromptResult const & )
	{
		CTrackerController::getInstance()->process_event( CSynchronizedWithNetworkEvent() );

		CSegmentFileStorage::getInstance()->resetState();
		CSegmentFileStorage::getInstance()->retriveState();

		context< CSynchronizationAction >().setRequest( 0 );

		return discard_event();
	}

	~CSynchronizingBlocks()
	{
		CSegmentFileStorage::getInstance()->releaseSynchronizationInProgress();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< CTransactionBlockEvent< CDiskBlock > >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckPromptResult >,
	boost::statechart::custom_reaction< common::CEndEvent >
	> reactions;
};


struct CSynchronizingHeaders : boost::statechart::state< CSynchronizingHeaders, CSynchronizationAction >
{
	CSynchronizingHeaders( my_context ctx ) : my_base( ctx )
	{
		CSegmentFileStorage::getInstance()->setSynchronizationInProgress();

		context< CSynchronizationAction >().setRequest(
					new CGetNextBlockRequest( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ), (int)CBlockKind::Header ) );
	}

	boost::statechart::result react( CTransactionBlockEvent< CSegmentHeader > const & _transactionBlockEvent )
	{
		CSegmentFileStorage::getInstance()->setDiscBlock( *_transactionBlockEvent.m_discBlock, _transactionBlockEvent.m_blockIndex );

		context< CSynchronizationAction >().setRequest(
					new CGetNextBlockRequest( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ), (int)CBlockKind::Header ) );

		return discard_event();
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CSynchronizationAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< CTransactionBlockEvent< CSegmentHeader > >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::transition< common::CEndEvent, CSynchronizingBlocks >
	> reactions;
};

struct CSynchronized : boost::statechart::state< CSynchronized, CSynchronizationAction >
{
	CSynchronized( my_context ctx ) : my_base( ctx ),m_currentBlock( 0 ),m_currentHeader( 0 )
	{
		CSegmentFileStorage::getInstance()->setSynchronizationInProgress();

		m_storedBlocks = CSegmentFileStorage::getInstance()->calculateStoredBlockNumber();

		m_storedHeaders = CSegmentFileStorage::getInstance()->getStoredHeaderCount();

		assert( m_storedBlocks );

		m_diskBlock = new CDiskBlock;

		m_segmentHeader = new CSegmentHeader;
		setHeaders();
	}

	void setBlock()
	{
		CSegmentFileStorage::getInstance()->getBlock( m_currentBlock, *m_diskBlock );

		context< CSynchronizationAction >().setRequest( new CSetNextBlockRequest< CDiskBlock >(
															context< CSynchronizationAction >().getActionKey()
															, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() )
															, m_diskBlock
															, m_currentBlock++) );
	}


	void setHeaders()
	{
		CSegmentFileStorage::getInstance()->getSegmentHeader( m_currentHeader, *m_segmentHeader );

		context< CSynchronizationAction >().setRequest( new CSetNextBlockRequest< CSegmentHeader >(
															context< CSynchronizationAction >().getActionKey()
															, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() )
															, m_segmentHeader
															, m_currentHeader++) );
	}

	boost::statechart::result react( common::CGetEvent const & _getEvent )
	{
		if ( m_currentBlock < m_storedBlocks && _getEvent.m_type == CBlockKind::Segment )
		{
			setBlock();
		}
		else if ( m_currentHeader < m_storedHeaders && _getEvent.m_type == CBlockKind::Header )
		{
			setHeaders();
		}
		else
		{
			context< CSynchronizationAction >().setRequest(
						new common::CEndRequest<TrackerResponses>( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
		}

		return discard_event();
	}

	boost::statechart::result react( common::CContinueEvent const & _continueEvent )
	{
		context< CSynchronizationAction >().setRequest( new common::CContinueReqest<TrackerResponses>( _continueEvent.m_keyId, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & )
	{
		context< CSynchronizationAction >().setRequest( 0 );

		return discard_event();
	}

	~CSynchronized()
	{
		delete m_diskBlock;
		delete m_segmentHeader;

		CSegmentFileStorage::getInstance()->releaseSynchronizationInProgress();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CGetEvent >,
	boost::statechart::custom_reaction< common::CContinueEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	unsigned int m_storedBlocks;
	unsigned int m_currentBlock;

	unsigned int m_storedHeaders;
	unsigned int m_currentHeader;

	CDiskBlock * m_diskBlock;
	CSegmentHeader * m_segmentHeader;
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
CSynchronizationAction::getRequest() const
{
	return m_request;
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
