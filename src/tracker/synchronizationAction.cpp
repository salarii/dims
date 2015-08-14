// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "common/setResponseVisitor.h"
#include "common/commonRequests.h"
#include "common/commonEvents.h"
#include "common/segmentFileStorage.h"
#include "common/mediumKinds.h"
#include "common/supportTransactionsDatabase.h"
#include "common/commonEvents.h"

#include "tracker/transactionRecordManager.h"
#include "tracker/synchronizationAction.h"
#include "tracker/trackerRequests.h"
#include "tracker/trackerFilters.h"
#include "tracker/trackerEvents.h"
#include "tracker/trackerController.h"
#include "tracker/trackerControllerEvents.h"

namespace tracker
{

unsigned const SynchronisingGetInfoTime = 10000;//milisec

unsigned const SynchronisingWaitTime = 15000;

struct CSynchronizingGetInfo;
struct CSynchronizedGetInfo;

struct CUninitiated : boost::statechart::simple_state< CUninitiated, CSynchronizationAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CSwitchToSynchronizing, CSynchronizingGetInfo >,
	boost::statechart::transition< CSwitchToSynchronized, CSynchronizedGetInfo >
	> reactions;
};

struct CSynchronizingHeaders;

struct CSynchronizingRegistrationAsk : boost::statechart::state< CSynchronizingGetInfo, CSynchronizationAction >
{
	CSynchronizingRegistrationAsk( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest(
					new common::CSynchronizationRequest< common::CTrackerTypes >(
						context< CSynchronizationAction >().getActionKey()
						, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						SynchronisingGetInfoTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return transit< CSynchronizingGetInfo >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};


struct CSynchronizingGetInfo : boost::statechart::state< CSynchronizingGetInfo, CSynchronizationAction >
{
	CSynchronizingGetInfo( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().dropRequests();

		context< CSynchronizationAction >().addRequest( new common::CGetSynchronizationInfoRequest< common::CTrackerTypes >(
															context< CSynchronizationAction >().getActionKey()
															, 0
															, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::SynchronizationInfo )
		{
			common::CSynchronizationInfo synchronizationInfo;

			common::convertPayload( orginalMessage, synchronizationInfo );

			context< CSynchronizationAction >().setStorageSize( synchronizationInfo.m_strageSize );
			context< CSynchronizationAction >().setHeaderSize( synchronizationInfo.m_headerSize );

			context< CSynchronizationAction >().addRequest(
						new common::CAckRequest< common::CTrackerTypes >(
							  context< CSynchronizationAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

		}
		return transit< CSynchronizingHeaders >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

struct CSynchronized;

struct CSynchronizedGetInfo : boost::statechart::state< CSynchronizedGetInfo, CSynchronizationAction >
{
	CSynchronizedGetInfo( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest( new common::CGetSynchronizationInfoRequest< common::CTrackerTypes >(
															context< CSynchronizationAction >().getActionKey()
															, common::CSegmentFileStorage::getInstance()->getTimeStampOfLastFlush()
															, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) )
														);
		context< CSynchronizationAction >().addRequest( new common::CTimeEventRequest< common::CTrackerTypes >( SynchronisingWaitTime * 2, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CGetEvent const & _getEvent )
	{
		return transit< CSynchronized >();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CSynchronizationAction >().dropRequests();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CGetEvent >
	> reactions;

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

		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest(
					new common::CGetNextBlockRequest< common::CTrackerTypes >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ), (int)common::CBlockKind::Segment ) );
	}

	boost::statechart::result react( common::CTransactionBlockEvent< common::CDiskBlock > const & _transactionBlockEvent )
	{
		std::vector< CTransaction > transactions;

		common::CSegmentFileStorage::getInstance()->setDiscBlock( *_transactionBlockEvent.m_discBlock, _transactionBlockEvent.m_blockIndex, transactions );

		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest(
					new common::CGetNextBlockRequest< common::CTrackerTypes >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ), (int)common::CBlockKind::Segment ) );

		BOOST_FOREACH( CTransaction const & transaction, transactions )
		{
			common::CSupportTransactionsDatabase::getInstance()->setTransactionLocation( transaction.GetHash(), transaction.m_location );
		}

		common::CSupportTransactionsDatabase::getInstance()->flush();

		CTransactionRecordManager::getInstance()->addValidatedTransactionBundle( transactions );

		return discard_event();
	}

	boost::statechart::result react( common::CEndEvent const & )
	{
		context< CSynchronizationAction >().dropRequests();
		//context< CSynchronizationAction >().addRequest( new common::CAckRequest< common::CTrackerTypes >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
		//generate  time  and  quit??
		CTrackerController::getInstance()->process_event( CSynchronizedWithNetworkEvent() );
		common::CSegmentFileStorage::getInstance()->resetState();
		common::CSegmentFileStorage::getInstance()->retriveState();

		return discard_event();
	}

	~CSynchronizingBlocks()
	{
		common::CSegmentFileStorage::getInstance()->releaseSynchronizationInProgress();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTransactionBlockEvent< common::CDiskBlock > >,
	boost::statechart::custom_reaction< common::CEndEvent >
	> reactions;
};


struct CSynchronizingHeaders : boost::statechart::state< CSynchronizingHeaders, CSynchronizationAction >
{
	CSynchronizingHeaders( my_context ctx ) : my_base( ctx )
	{
		common::CSegmentFileStorage::getInstance()->setSynchronizationInProgress();

		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest(
					new common::CGetNextBlockRequest< common::CTrackerTypes >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ), (int)common::CBlockKind::Header ) );
	}

	boost::statechart::result react( common::CTransactionBlockEvent< common::CSegmentHeader > const & _transactionBlockEvent )
	{
		common::CSegmentFileStorage::getInstance()->setDiscBlock( *_transactionBlockEvent.m_discBlock, _transactionBlockEvent.m_blockIndex );

		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest(
					new common::CGetNextBlockRequest< common::CTrackerTypes >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ), (int)common::CBlockKind::Header ) );

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTransactionBlockEvent< common::CSegmentHeader > >,
	boost::statechart::transition< common::CEndEvent, CSynchronizingBlocks >
	> reactions;
};

struct CSynchronized : boost::statechart::state< CSynchronized, CSynchronizationAction >
{
	CSynchronized( my_context ctx ) : my_base( ctx ),m_currentBlock( 0 ),m_currentHeader( 0 )
	{
		common::CSegmentFileStorage::getInstance()->setSynchronizationInProgress();

		m_storedBlocks = common::CSegmentFileStorage::getInstance()->calculateStoredBlockNumber();

		m_storedHeaders = common::CSegmentFileStorage::getInstance()->getStoredHeaderCount();

		assert( m_storedBlocks );

		m_diskBlock = new common::CDiskBlock;

		m_segmentHeader = new common::CSegmentHeader;
		setHeaders();
	}

	void setBlock()
	{
		common::CSegmentFileStorage::getInstance()->getBlock( m_currentBlock, *m_diskBlock );

		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest( new common::CSetNextBlockRequest< common::CDiskBlock, common::CTrackerTypes >(
															context< CSynchronizationAction >().getActionKey()
															, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() )
															, m_diskBlock
															, m_currentBlock++) );
	}


	void setHeaders()
	{
		common::CSegmentFileStorage::getInstance()->getSegmentHeader( m_currentHeader, *m_segmentHeader );

		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest( new common::CSetNextBlockRequest< common::CSegmentHeader, common::CTrackerTypes >(
															context< CSynchronizationAction >().getActionKey()
															, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() )
															, m_segmentHeader
															, m_currentHeader++) );
	}

	boost::statechart::result react( common::CGetEvent const & _getEvent )
	{
		if ( m_currentBlock < m_storedBlocks && _getEvent.m_type == common::CBlockKind::Segment )
		{
			setBlock();
		}
		else if ( m_currentHeader < m_storedHeaders && _getEvent.m_type == common::CBlockKind::Header )
		{
			setHeaders();
		}
		else
		{
			context< CSynchronizationAction >().dropRequests();
			context< CSynchronizationAction >().addRequest(
						new common::CEndRequest< common::CTrackerTypes >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
		}

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & )
	{
		context< CSynchronizationAction >().dropRequests();

		return discard_event();
	}

	~CSynchronized()
	{
		delete m_diskBlock;
		delete m_segmentHeader;

		common::CSegmentFileStorage::getInstance()->releaseSynchronizationInProgress();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CGetEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	unsigned int m_storedBlocks;
	unsigned int m_currentBlock;

	unsigned int m_storedHeaders;
	unsigned int m_currentHeader;

	common::CDiskBlock * m_diskBlock;
	common::CSegmentHeader * m_segmentHeader;
};

CSynchronizationAction::CSynchronizationAction( uintptr_t _nodeIndicator )
	: m_nodeIdentifier( _nodeIndicator )
{
	initiate();
}

CSynchronizationAction::CSynchronizationAction( uint256 const & _actionKey, uintptr_t _nodeIndicator, uint64_t _timeStamp )
	: m_timeStamp( _timeStamp )
	, m_nodeIdentifier( _nodeIndicator )
{
	initiate();
	process_event( CSwitchToSynchronized() );
}

void
CSynchronizationAction::accept( common::CSetResponseVisitor< common::CTrackerTypes > & _visitor )
{
	_visitor.visit( *this );
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
	return !m_requests.empty();
}

void
CSynchronizationAction::setNodeIdentifier( unsigned int _nodeIdentifier )
{
	m_nodeIdentifier = _nodeIdentifier;
}

}

//struct CDiskBlock;
