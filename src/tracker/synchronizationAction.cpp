// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "wallet.h"

#include "common/setResponseVisitor.h"
#include "common/commonRequests.h"
#include "common/commonEvents.h"
#include "common/segmentFileStorage.h"
#include "common/mediumKinds.h"
#include "common/supportTransactionsDatabase.h"
#include "common/commonEvents.h"
#include "common/actionHandler.h"
#include "common/analyseTransaction.h"

#include "tracker/transactionRecordManager.h"
#include "tracker/synchronizationAction.h"
#include "tracker/trackerRequests.h"
#include "tracker/trackerFilters.h"
#include "tracker/trackerEvents.h"
#include "tracker/trackerController.h"
#include "tracker/trackerControllerEvents.h"
#include "tracker/trackOriginAddressAction.h"

namespace tracker
{

unsigned const SynchronisingGetInfoTime = 10000;//milisec

unsigned const SynchronisingWaitTime = 15000;

struct CSynchronizingGetInfo;
struct CSynchronizedGetInfo;
struct CSynchronizingRegistrationAsk;
struct CGetBitcoinHeader;

struct CUninitiated : boost::statechart::simple_state< CUninitiated, CSynchronizationAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CSwitchToSynchronizing, CSynchronizingRegistrationAsk >,
	boost::statechart::transition< CSwitchToSynchronized, CSynchronizedGetInfo >
	> reactions;
};

struct CSynchronizingHeaders;

struct CSynchronizingRegistrationAsk : boost::statechart::state< CSynchronizingRegistrationAsk, CSynchronizationAction >
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
		CTransactionRecordManager::getInstance()->clearCoinViewDB();
		CTransactionRecordManager::getInstance()->clearAddressToCoinsDatabase();
		CTransactionRecordManager::getInstance()->clearSupportTransactionsDatabase();


		CWallet::getInstance()->resetDatabase();

		common::CSegmentFileStorage::getInstance()->setSynchronizationInProgress();

		return transit< CGetBitcoinHeader >();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CGetBitcoinHeader: boost::statechart::state< CGetBitcoinHeader, CSynchronizationAction >
{
	CGetBitcoinHeader( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().addRequest(
					new common::CInfoAskRequest< common::CTrackerTypes >(
						common::CInfoKind::BitcoinHeaderAsk
						, context< CSynchronizationAction >().getActionKey()
						, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						 SynchronisingGetInfoTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}


	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::SynchronizationBitcoinHeader )
		{
			common::CBitcoinHeader bitcoinHeader;

			common::convertPayload( orginalMessage, bitcoinHeader );

			CAutoFile file( OpenHeadFile(false), SER_DISK, CLIENT_VERSION );

			file << bitcoinHeader.m_bitcoinHeader;
			fflush(file);
			FileCommit(file);

			context< CSynchronizationAction >().addRequest(
						new common::CAckRequest< common::CTrackerTypes >(
							  context< CSynchronizationAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

			common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( new tracker::CTrackOriginAddressAction );

		}
		return transit< CSynchronizingGetInfo >();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CSynchronizationAction >().dropRequests();
		context< CSynchronizationAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
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

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CSynchronizationAction >().dropRequests();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;

	// best synchronising option
	uint64_t m_bestTimeStamp;
};

struct CSynchronizingBlocks : boost::statechart::state< CSynchronizingBlocks, CSynchronizationAction >
{
	CSynchronizingBlocks( my_context ctx ) : my_base( ctx ), m_currentBlock( 0 )
	{
		context< CSynchronizationAction >().dropRequests();

		context< CSynchronizationAction >().addRequest(
					new common::CGetBlockRequest< common::CTrackerTypes >(
						  m_currentBlock
						, (int)common::CBlockKind::Segment
						, context< CSynchronizationAction >().getActionKey()
						, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );


		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						  SynchronisingWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::SynchronizationBlock )
		{
			common::CSynchronizationBlock synchronizationBlock( new common::CDiskBlock(), -1 );

			common::convertPayload( orginalMessage, synchronizationBlock );
			context< CSynchronizationAction >().dropRequests();

			std::vector< CTransaction > transactions;

			assert( synchronizationBlock.m_blockIndex == m_currentBlock );

			common::CSegmentFileStorage::getInstance()->setDiscBlock( *synchronizationBlock.m_diskBlock, synchronizationBlock.m_blockIndex, transactions );

			BOOST_FOREACH( CTransaction const & transaction, transactions )
			{
				common::findSelfCoinsAndAddToWallet( transaction );
				common::CSupportTransactionsDatabase::getInstance()->setTransactionLocation( transaction.GetHash(), transaction.m_location );
			}

			common::CSupportTransactionsDatabase::getInstance()->flush();

			CTransactionRecordManager::getInstance()->addRetrivedTransactionBundle( transactions );

			if ( context< CSynchronizationAction >().getStorageSize() > ++m_currentBlock )
			{
				context< CSynchronizationAction >().addRequest(
							new common::CGetBlockRequest< common::CTrackerTypes >(
								m_currentBlock
								, (int)common::CBlockKind::Segment
								, context< CSynchronizationAction >().getActionKey()
								, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
			}
			else
			{
				CTrackerController::getInstance()->process_event( CSynchronizedWithNetworkEvent() );
				common::CSegmentFileStorage::getInstance()->resetState();
				common::CSegmentFileStorage::getInstance()->retriveState();

				common::CSynchronizationResult synchronizationResult;
				synchronizationResult.m_result = 1;

				context< CSynchronizationAction >().setResult( synchronizationResult );
				context< CSynchronizationAction >().setExit();
			}
		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CSynchronizationAction >().dropRequests();

		context< CSynchronizationAction >().addRequest(
					new common::CGetBlockRequest< common::CTrackerTypes >(
						  m_currentBlock
						, (int)common::CBlockKind::Segment
						, context< CSynchronizationAction >().getActionKey()
						, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );


		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						  SynchronisingWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return discard_event();
	}

	~CSynchronizingBlocks()
	{
		common::CSegmentFileStorage::getInstance()->releaseSynchronizationInProgress();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

	unsigned int m_currentBlock;
};


struct CSynchronizingHeaders : boost::statechart::state< CSynchronizingHeaders, CSynchronizationAction >
{
	CSynchronizingHeaders( my_context ctx ) : my_base( ctx ), m_currentBlock( 0 )
	{
		context< CSynchronizationAction >().dropRequests();

		context< CSynchronizationAction >().addRequest(
					new common::CGetBlockRequest< common::CTrackerTypes >(
						  m_currentBlock
						, (int)common::CBlockKind::Header
						, context< CSynchronizationAction >().getActionKey()
						, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );


		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						  SynchronisingWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::SynchronizationHeader )
		{
			common::CSynchronizationSegmentHeader synchronizationHeader( new common::CSegmentHeader(), -1 );

			common::convertPayload( orginalMessage, synchronizationHeader );
			context< CSynchronizationAction >().dropRequests();

			common::CSegmentFileStorage::getInstance()->setDiscBlock( *synchronizationHeader.m_segmentHeader, synchronizationHeader.m_blockIndex );

			context< CSynchronizationAction >().addRequest(
						new common::CAckRequest< common::CTrackerTypes >(
							  context< CSynchronizationAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

			if ( context< CSynchronizationAction >().getHeaderSize() > ++m_currentBlock )
			{
				context< CSynchronizationAction >().addRequest(
							new common::CGetBlockRequest< common::CTrackerTypes >(
								m_currentBlock
								, (int)common::CBlockKind::Header
								, context< CSynchronizationAction >().getActionKey()
								, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );
			}
			else
			{
				return transit< CSynchronizingBlocks >();
			}

			assert( synchronizationHeader.m_blockIndex == m_currentBlock - 1 );
		}
		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CSynchronizationAction >().dropRequests();

		context< CSynchronizationAction >().addRequest(
					new common::CGetBlockRequest< common::CTrackerTypes >(
						  m_currentBlock
						, (int)common::CBlockKind::Header
						, context< CSynchronizationAction >().getActionKey()
						, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );


		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest< common::CTrackerTypes >(
						  SynchronisingWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CAckEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;

	unsigned int m_currentBlock;
};

struct CSynchronized : boost::statechart::state< CSynchronized, CSynchronizationAction >
{
	CSynchronized( my_context ctx ) : my_base( ctx ),m_currentBlock( 0 ),m_currentHeader( 0 )
	{

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
	process_event( CSwitchToSynchronizing() );
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
