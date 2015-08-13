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

#include "monitor/synchronizationAction.h"
#include "monitor/filters.h"
#include "monitor/copyTransactionStorageAction.h"
#include "monitor/copyStorageHandler.h"

namespace monitor
{

unsigned const SynchronisingGetInfoTime = 10000;//milisec

unsigned const SynchronisingWaitTime = 15000;

struct CSynchronizingGetInfo;
struct CSynchronizedGetInfo;

struct CSwitchToSynchronizing : boost::statechart::event< CSwitchToSynchronizing >
{
};

struct CSwitchToSynchronized : boost::statechart::event< CSwitchToSynchronized >
{
};

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
	CSynchronizingGetInfo( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest( new common::CGetSynchronizationInfoRequest< common::CMonitorTypes >( context< CSynchronizationAction >().getActionKey(), 0, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
		context< CSynchronizationAction >().addRequest( new common::CTimeEventRequest< common::CMonitorTypes >( SynchronisingGetInfoTime, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
		m_bestTimeStamp = 0;
	}

	boost::statechart::result react( common::CSynchronizationInfoEvent const & _synchronizationInfoEvent )
	{
		if ( m_bestTimeStamp < _synchronizationInfoEvent.m_timeStamp )
		{
			m_bestTimeStamp = _synchronizationInfoEvent.m_timeStamp;
			context< CSynchronizationAction >().setNodeIdentifier( _synchronizationInfoEvent.m_nodeIdentifier );
		}
		context< CSynchronizationAction >().dropRequests();
		return discard_event();
	}

boost::statechart::result react( common::CTimeEvent const & _timeEvent )
{
	if ( m_bestTimeStamp > 0 )
	{
		context< CSynchronizationAction >().dropRequests();
		return transit< CSynchronizingHeaders >();
	}
	else
	{
		context< CSynchronizationAction >().dropRequests();
		return discard_event();
	}
}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CSynchronizationInfoEvent >
	> reactions;

	// best synchronising option
	uint64_t m_bestTimeStamp;
};

struct CSynchronized;

struct CSynchronizedGetInfo : boost::statechart::state< CSynchronizedGetInfo, CSynchronizationAction >
{
	CSynchronizedGetInfo( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest( new common::CGetSynchronizationInfoRequest< common::CMonitorTypes >(
															context< CSynchronizationAction >().getActionKey()
															, common::CSegmentFileStorage::getInstance()->getTimeStampOfLastFlush()
															, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) )
														);

		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest< common::CMonitorTypes >(
						  SynchronisingWaitTime * 2
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
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
		/*
		CTransactionRecordManager::getInstance()->clearCoinViewDB();
		CTransactionRecordManager::getInstance()->clearAddressToCoinsDatabase();
		CTransactionRecordManager::getInstance()->clearSupportTransactionsDatabase();
*/
		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest(
					new common::CGetNextBlockRequest< common::CMonitorTypes >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ), (int)common::CBlockKind::Segment ) );
	}

	boost::statechart::result react( common::CTransactionBlockEvent< common::CDiskBlock > const & _transactionBlockEvent )
	{
		std::vector< CTransaction > transactions;

		common::CSegmentFileStorage::getInstance()->setDiscBlock( *_transactionBlockEvent.m_discBlock, _transactionBlockEvent.m_blockIndex, transactions );

		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest(
					new common::CGetNextBlockRequest< common::CMonitorTypes >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ), (int)common::CBlockKind::Segment ) );

		BOOST_FOREACH( CTransaction const & transaction, transactions )
		{
			common::CSupportTransactionsDatabase::getInstance()->setTransactionLocation( transaction.GetHash(), transaction.m_location );
		}

		common::CSupportTransactionsDatabase::getInstance()->flush();

	//	CTransactionRecordManager::getInstance()->addValidatedTransactionBundle( transactions );

		return discard_event();
	}

	boost::statechart::result react( common::CEndEvent const & )
	{
		context< CSynchronizationAction >().dropRequests();
		//context< CSynchronizationAction >().addRequest( new common::CAckRequest< common::CTrackerTypes >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
		//generate  time  and  quit??
		//CTrackerController::getInstance()->process_event( CSynchronizedWithNetworkEvent() );
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
					new common::CGetNextBlockRequest< common::CMonitorTypes >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ), (int)common::CBlockKind::Header ) );
	}

	boost::statechart::result react( common::CTransactionBlockEvent< common::CSegmentHeader > const & _transactionBlockEvent )
	{
		common::CSegmentFileStorage::getInstance()->setDiscBlock( *_transactionBlockEvent.m_discBlock, _transactionBlockEvent.m_blockIndex );

		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest(
					new common::CGetNextBlockRequest< common::CMonitorTypes >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ), (int)common::CBlockKind::Header ) );

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTransactionBlockEvent< common::CSegmentHeader > >,
	boost::statechart::transition< common::CEndEvent, CSynchronizingBlocks >
	> reactions;
};

struct CSynchronizedUninitialized : boost::statechart::state< CSynchronizedUninitialized, CSynchronizationAction >
{
	CSynchronizedUninitialized( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest< common::CMonitorTypes >(
						SynchronisingWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
		{
			common::CInfoRequestData infoRequest;

			common::convertPayload( orginalMessage, infoRequest );

			if ( infoRequest.m_kind == common::CInfoKind::StorageInfoAsk )
			{
				context< CSynchronizationAction >().dropRequests();

				context< CSynchronizationAction >().setRequestKey( _messageResult.m_message.m_header.m_id );

				context< CSynchronizationAction >().addRequest(
							new common::CAckRequest< common::CMonitorTypes >(
								context< CSynchronizationAction >().getActionKey()
								, _messageResult.m_message.m_header.m_id
								, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
			}

		}

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;
};

struct CSynchronizedProvideCopy : boost::statechart::state< CSynchronized, CSynchronizationAction >
{
	CSynchronizedProvideCopy( my_context ctx ) : my_base( ctx ), m_copyRequestDone( false )
	{
		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest< common::CMonitorTypes >(
						SynchronisingWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		if ( m_copyRequestDone )
		{
			m_copyRequestDone = CCopyStorageHandler::getInstance()->createCopyRequest();
		}
		else
		{
			if ( CCopyStorageHandler::getInstance()->copyCreated() )
			{
						context< CSynchronizationAction >().addRequest(
									new common::CStorageInfoRequest< common::CMonitorTypes >(
										  CCopyStorageHandler::getInstance()->getTimeStamp()
										, CCopyStorageHandler::getInstance()->getDiscBlockSize()
										, CCopyStorageHandler::getInstance()->getSegmentHeaderSize()
										, context< CSynchronizationAction >().getActionKey()
										, context< CSynchronizationAction >().getRequestKey()
										, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
			}
			else
			{
				context< CSynchronizationAction >().addRequest(
							new common::CTimeEventRequest< common::CMonitorTypes >(
								SynchronisingWaitTime
								, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
			}
		}

			return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
		boost::statechart::transition< common::CAckEvent, CSynchronized >
	> reactions;

	bool m_copyRequestDone;
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
		context< CSynchronizationAction >().addRequest( new common::CSetNextBlockRequest< common::CDiskBlock, common::CMonitorTypes >(
															context< CSynchronizationAction >().getActionKey()
															, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() )
															, m_diskBlock
															, m_currentBlock++) );
	}


	void setHeaders()
	{
		common::CSegmentFileStorage::getInstance()->getSegmentHeader( m_currentHeader, *m_segmentHeader );

		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest( new common::CSetNextBlockRequest< common::CSegmentHeader, common::CMonitorTypes >(
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
						new common::CEndRequest< common::CMonitorTypes >( context< CSynchronizationAction >().getActionKey(), new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
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

CSynchronizationAction::CSynchronizationAction()
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
CSynchronizationAction::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
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
