// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "wallet.h"

#include "common/analyseTransaction.h"
#include "common/actionHandler.h"
#include "common/authenticationProvider.h"
#include "common/setResponseVisitor.h"
#include "common/requests.h"
#include "common/events.h"
#include "common/segmentFileStorage.h"
#include "common/mediumKinds.h"
#include "common/supportTransactionsDatabase.h"
#include "common/events.h"

#include "monitor/synchronizationAction.h"
#include "monitor/filters.h"
#include "monitor/copyStorageHandler.h"
#include "monitor/transactionRecordManager.h"
#include "monitor/trackOriginAddressAction.h"

namespace monitor
{

unsigned const SynchronisingGetInfoTime = 10000;//milisec

unsigned const SynchronisingWaitTime = 15000;

struct CSynchronizingGetInfo;
struct CSynchronizingHeaders;
struct CSynchronizedUninitialized;
struct CSynchronizedProvideCopy;
struct CSynchronized;
struct CSynchronizingAsk;
struct CGetBitcoinHeader;

namespace
{
unsigned int HeaderSize;
unsigned int StrageSize;
}

struct CSwitchToSynchronizing : boost::statechart::event< CSwitchToSynchronizing >
{
};

struct CUninitiated : boost::statechart::simple_state< CUninitiated, CSynchronizationAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CSwitchToSynchronizing, CSynchronizingAsk >,
	boost::statechart::transition< CSwitchToSynchronized, CSynchronizedUninitialized >
	> reactions;
};


struct CSynchronizingAsk : boost::statechart::state< CSynchronizingAsk, CSynchronizationAction >
{
	CSynchronizingAsk( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().forgetRequests();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::SynchronizationAsk
					, context< CSynchronizationAction >().getActionKey()
					, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) );

		request->addPayload( common::CSynchronizationAsk() );

		context< CSynchronizationAction >().addRequest( request );

		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest(
						 SynchronisingGetInfoTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CSynchronizationAction >().forgetRequests();
		context< CSynchronizationAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		CWallet::getInstance()->resetDatabase();

		CWallet::getInstance()->AddKeyPubKey(
					common::CAuthenticationProvider::getInstance()->getMyPrivKey()
					, common::CAuthenticationProvider::getInstance()->getMyKey());

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
		context< CSynchronizationAction >().forgetRequests();
		context< CSynchronizationAction >().addRequest(
					new common::CInfoAskRequest(
						common::CInfoKind::BitcoinHeaderAsk
						, context< CSynchronizationAction >().getActionKey()
						, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest(
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

			resetChains();
			context< CSynchronizationAction >().addRequest(
						new common::CAckRequest(
							  context< CSynchronizationAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

			common::CActionHandler::getInstance()->executeAction( new CTrackOriginAddressAction );

		}
		return transit< CSynchronizingGetInfo >();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CSynchronizationAction >().forgetRequests();
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
		context< CSynchronizationAction >().forgetRequests();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, context< CSynchronizationAction >().getActionKey()
					, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) );

		request->addPayload( (int)common::CInfoKind::StorageInfoAsk, 0 );

		context< CSynchronizationAction >().addRequest( request );
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

			StrageSize = synchronizationInfo.m_strageSize;
			HeaderSize = synchronizationInfo.m_headerSize;

			context< CSynchronizationAction >().addRequest(
						new common::CAckRequest(
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

struct CSynchronizingBlocks : boost::statechart::state< CSynchronizingBlocks, CSynchronizationAction >
{
	CSynchronizingBlocks( my_context ctx ) : my_base( ctx ), m_currentBlock( 0 )
	{
		context< CSynchronizationAction >().forgetRequests();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::SynchronizationGet
					, context< CSynchronizationAction >().getActionKey()
					, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) );

		request->addPayload(
					common::CSynchronizationGet(
						(int)common::CBlockKind::Segment
						, m_currentBlock ) );

		context< CSynchronizationAction >().addRequest( request );

		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest(
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
			context< CSynchronizationAction >().forgetRequests();

			std::vector< CTransaction > transactions;

			assert( synchronizationBlock.m_blockIndex == m_currentBlock );

			common::CSegmentFileStorage::getInstance()->setDiscBlock( *synchronizationBlock.m_diskBlock, synchronizationBlock.m_blockIndex, transactions );

			BOOST_FOREACH( CTransaction const & transaction, transactions )
			{
				common::findSelfCoinsAndAddToWallet( transaction );
				common::CSupportTransactionsDatabase::getInstance()->setTransactionLocation( transaction.GetHash(), transaction.m_location );
			}

			common::CSupportTransactionsDatabase::getInstance()->flush();

			if ( StrageSize > ++m_currentBlock )
			{
				common::CSendMessageRequest * request =
						new common::CSendMessageRequest(
							common::CPayloadKind::SynchronizationGet
							, context< CSynchronizationAction >().getActionKey()
							, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) );

				request->addPayload(
							common::CSynchronizationGet(
								(int)common::CBlockKind::Segment
								, m_currentBlock ) );

				context< CSynchronizationAction >().addRequest( request );
			}
			else
			{
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
		context< CSynchronizationAction >().forgetRequests();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::SynchronizationGet
					, context< CSynchronizationAction >().getActionKey()
					, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) );

		request->addPayload(
					common::CSynchronizationGet(
						(int)common::CBlockKind::Segment
						, m_currentBlock ) );

		context< CSynchronizationAction >().addRequest( request );

		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest(
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
		context< CSynchronizationAction >().forgetRequests();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::SynchronizationGet
					, context< CSynchronizationAction >().getActionKey()
					, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) );

		request->addPayload(
					common::CSynchronizationGet(
						(int)common::CBlockKind::Header
						, m_currentBlock ) );

		context< CSynchronizationAction >().addRequest( request );

		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest(
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
			context< CSynchronizationAction >().forgetRequests();

			common::CSegmentFileStorage::getInstance()->setDiscBlock( *synchronizationHeader.m_segmentHeader, synchronizationHeader.m_blockIndex );

			context< CSynchronizationAction >().addRequest(
						new common::CAckRequest(
							  context< CSynchronizationAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

			if ( HeaderSize > ++m_currentBlock )
			{
				common::CSendMessageRequest * request =
						new common::CSendMessageRequest(
							common::CPayloadKind::SynchronizationGet
							, context< CSynchronizationAction >().getActionKey()
							, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) );

				request->addPayload(
							common::CSynchronizationGet(
								(int)common::CBlockKind::Header
								, m_currentBlock ) );

				context< CSynchronizationAction >().addRequest( request );
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
		context< CSynchronizationAction >().forgetRequests();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::SynchronizationGet
					, context< CSynchronizationAction >().getActionKey()
					, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) );

		request->addPayload(
					common::CSynchronizationGet(
						(int)common::CBlockKind::Header
						, m_currentBlock ) );

		context< CSynchronizationAction >().addRequest( request );

		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest(
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

struct CSynchronizedUninitialized : boost::statechart::state< CSynchronizedUninitialized, CSynchronizationAction >
{
	CSynchronizedUninitialized( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest(
						SynchronisingWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CSynchronizationAction >().addRequest(
					new common::CAckRequest(
						context< CSynchronizationAction >().getActionKey()
						, context< CSynchronizationAction >().getRequestKey()
						, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

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
				context< CSynchronizationAction >().forgetRequests();

				context< CSynchronizationAction >().setRequestKey( _messageResult.m_message.m_header.m_id );

				context< CSynchronizationAction >().addRequest(
							new common::CAckRequest(
								context< CSynchronizationAction >().getActionKey()
								, _messageResult.m_message.m_header.m_id
								, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

				context< CSynchronizationAction >().setRequestKey( _messageResult.m_message.m_header.m_id );

				return transit< CSynchronizedProvideCopy >();
			}
			else if ( infoRequest.m_kind == common::CInfoKind::BitcoinHeaderAsk )
			{
				context< CSynchronizationAction >().addRequest(
							new common::CAckRequest(
								context< CSynchronizationAction >().getActionKey()
								, _messageResult.m_message.m_header.m_id
								, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

				CAutoFile file(OpenHeadFile(true), SER_DISK, CLIENT_VERSION);
				CBlockHeader header;
				file >> header;

				common::CSendMessageRequest * request =
						new common::CSendMessageRequest(
							common::CPayloadKind::SynchronizationBitcoinHeader
							, context< CSynchronizationAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) );

				request->addPayload( header );

				context< CSynchronizationAction >().addRequest( request );
			}
		}

		return discard_event();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CSynchronizationAction >().setExit();
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CTimeEvent >
	> reactions;
};

struct CSynchronizedProvideCopy : boost::statechart::state< CSynchronizedProvideCopy, CSynchronizationAction >
{
	CSynchronizedProvideCopy( my_context ctx ) : my_base( ctx ), m_copyRequestDone( false )
	{
		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest(
						 100
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		if ( !m_copyRequestDone )
		{
			m_copyRequestDone = CCopyStorageHandler::getInstance()->createCopyRequest();
		}
		else
		{
			if ( CCopyStorageHandler::getInstance()->copyCreated() )
			{
				common::CSendMessageRequest * request =
						new common::CSendMessageRequest(
							common::CPayloadKind::SynchronizationInfo
							, context< CSynchronizationAction >().getActionKey()
							, context< CSynchronizationAction >().getRequestKey()
							, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) );

				request->addPayload(
							common::CSynchronizationInfo(
										 CCopyStorageHandler::getInstance()->getTimeStamp()
										 , CCopyStorageHandler::getInstance()->getSegmentHeaderSize()
										 , CCopyStorageHandler::getInstance()->getDiscBlockSize() ) );

				context< CSynchronizationAction >().addRequest( request );
			}
		}

		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest(
						SynchronisingWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

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
	CSynchronized( my_context ctx ) : my_base( ctx )
	{

		m_storedBlocks = CCopyStorageHandler::getInstance()->getDiscBlockSize();

		m_storedHeaders = CCopyStorageHandler::getInstance()->getSegmentHeaderSize();

		assert( m_storedBlocks );

		m_diskBlock = new common::CDiskBlock;

		m_segmentHeader = new common::CSegmentHeader;
	}

	void setBlock( unsigned int _blockNumber )
	{
		common::CSegmentFileStorage::getInstance()->getCopyBlock( _blockNumber, *m_diskBlock );

		context< CSynchronizationAction >().forgetRequests();

		common::CSynchronizationBlock synchronizationBlock( m_diskBlock, _blockNumber );

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::SynchronizationBlock,
					context< CSynchronizationAction >().getActionKey()
					, m_id
					, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) );

		request->addPayload( synchronizationBlock );

		context< CSynchronizationAction >().addRequest( request );

	}

	void setHeaders( unsigned int _headerNumber )
	{
		common::CSegmentFileStorage::getInstance()->getCopySegmentHeader( _headerNumber, *m_segmentHeader );

		common::CSynchronizationSegmentHeader synchronizationSegmentHeader( m_segmentHeader, _headerNumber );

		context< CSynchronizationAction >().forgetRequests();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::SynchronizationHeader,
					context< CSynchronizationAction >().getActionKey()
					, m_id
					, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) );

		request->addPayload( synchronizationSegmentHeader );

		context< CSynchronizationAction >().addRequest( request );
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::SynchronizationGet )
		{
			m_id = orginalMessage.m_header.m_id;

			common::CSynchronizationGet synchronizationGet;

			common::convertPayload( orginalMessage, synchronizationGet );

			context< CSynchronizationAction >().addRequest(
						new common::CAckRequest(
							context< CSynchronizationAction >().getActionKey()
							, m_id
							, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

			if ( synchronizationGet.m_number < m_storedBlocks && synchronizationGet.m_kind == common::CBlockKind::Segment )
			{
				setBlock( synchronizationGet.m_number );
			}
			else if ( synchronizationGet.m_number < m_storedHeaders && synchronizationGet.m_kind == common::CBlockKind::Header )
			{
				setHeaders( synchronizationGet.m_number );
			}
		}
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & )
	{
		context< CSynchronizationAction >().forgetRequests();

		return discard_event();
	}

	~CSynchronized()
	{
		delete m_diskBlock;
		delete m_segmentHeader;
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;

	unsigned int m_storedBlocks;

	unsigned int m_storedHeaders;

	common::CDiskBlock * m_diskBlock;
	common::CSegmentHeader * m_segmentHeader;

	uint256 m_id;
};

CSynchronizationAction::CSynchronizationAction( uintptr_t _nodeIndicator )
{
	initiate();
}

CSynchronizationAction::CSynchronizationAction( uint256 const & _id, uint256 const & _actionKey, uintptr_t _nodeIndicator )
	: common::CScheduleAbleAction( _actionKey )
	, m_requestKey( _id )
	, m_nodeIdentifier( _nodeIndicator )
{
	initiate();
}

void
CSynchronizationAction::accept( common::CSetResponseVisitor & _visitor )
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
