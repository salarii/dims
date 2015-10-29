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
#include "monitor/reputationControlAction.h"

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


struct CSwitchToSynchronized : boost::statechart::event< CSwitchToSynchronized >
{
};

struct CSwitchToSynchronizing : boost::statechart::event< CSwitchToSynchronizing >
{
};

struct CUninitiatedSynchronization : boost::statechart::simple_state< CUninitiatedSynchronization, CSynchronizationAction >
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
					, new CByKeyMediumFilter( context< CSynchronizationAction >().getPartnerKey() ) );

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
		context< CSynchronizationAction >().setResult( common::CSynchronizationResult( 0 ) );
		context< CSynchronizationAction >().setExit();
		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & _promptAck )
	{
		return discard_event();
	}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey) )
			assert( !"service it somehow" );

		if ( orginalMessage.m_header.m_payloadKind == common::CPayloadKind::Result )
		{
			common::CResult result;

			common::convertPayload( orginalMessage, result );

			context< CSynchronizationAction >().forgetRequests();

			context< CSynchronizationAction >().addRequest(
						new common::CAckRequest(
							  context< CSynchronizationAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

			if ( result.m_result )
			{
				CWallet::getInstance()->resetDatabase();

				CWallet::getInstance()->AddKeyPubKey(
							common::CAuthenticationProvider::getInstance()->getMyPrivKey()
							, common::CAuthenticationProvider::getInstance()->getMyKey());

				common::CSegmentFileStorage::getInstance()->setSynchronizationInProgress();

				return transit< CGetBitcoinHeader >();
			}
			context< CSynchronizationAction >().setResult( common::CSynchronizationResult( 0 ) );
			context< CSynchronizationAction >().setExit();
		}
		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CTimeEvent >,
	boost::statechart::custom_reaction< common::CMessageResult >,
	boost::statechart::custom_reaction< common::CAckEvent >
	> reactions;
};

struct CGetBitcoinHeader: boost::statechart::state< CGetBitcoinHeader, CSynchronizationAction >
{
	CGetBitcoinHeader( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().forgetRequests();

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::InfoReq
					, context< CSynchronizationAction >().getActionKey()
					, new CByKeyMediumFilter( context< CSynchronizationAction >().getPartnerKey() ) );

		request->addPayload( common::CInfoRequestData( (int)common::CInfoKind::BitcoinHeaderAsk, std::vector<unsigned char>() ) );

		context< CSynchronizationAction >().addRequest( request );

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
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

			common::CActionHandler::getInstance()->executeAction( CTrackOriginAddressAction::createInstance() );

		}
		return transit< CSynchronizingGetInfo >();
	}

	boost::statechart::result react( common::CTimeEvent const & _timeEvent )
	{
		context< CSynchronizationAction >().forgetRequests();
		context< CSynchronizationAction >().setResult( common::CSynchronizationResult( 0 ) );
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
					, new CByKeyMediumFilter( context< CSynchronizationAction >().getPartnerKey() ) );

		request->addPayload( common::CInfoRequestData( (int)common::CInfoKind::StorageInfoAsk, 0 ) );

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
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

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
					, new CByKeyMediumFilter( context< CSynchronizationAction >().getPartnerKey() ) );

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
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) );

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

				if ( !CReputationTracker::getInstance()->isRegisteredTracker( _messageResult.m_pubKey.GetID() ) )
					CReputationTracker::getInstance()->removeNodeFromSynch( _messageResult.m_pubKey.GetID() );

				context< CSynchronizationAction >().setResult( common::CSynchronizationResult( 1 ) );
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
					, new CByKeyMediumFilter( context< CSynchronizationAction >().getPartnerKey() ) );

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
					, new CByKeyMediumFilter( context< CSynchronizationAction >().getPartnerKey() ) );

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
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

			if ( HeaderSize > ++m_currentBlock )
			{
				common::CSendMessageRequest * request =
						new common::CSendMessageRequest(
							common::CPayloadKind::SynchronizationGet
							, context< CSynchronizationAction >().getActionKey()
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) );

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
					, new CByKeyMediumFilter( context< CSynchronizationAction >().getPartnerKey() ) );

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
						, new CByKeyMediumFilter( context< CSynchronizationAction >().getPartnerKey() ) ) );

		common::CSendMessageRequest * request =
				new common::CSendMessageRequest(
					common::CPayloadKind::Result
					, context< CSynchronizationAction >().getActionKey()
					, context< CSynchronizationAction >().getRequestKey()
					, new CByKeyMediumFilter( context< CSynchronizationAction >().getPartnerKey() ) );

		bool allowed = CReputationTracker::getInstance()->isSynchronizationAllowed( context< CSynchronizationAction >().getPartnerKey().GetID() );

		request->addPayload(
					common::CResult( allowed ? 1 : 0 ) );

		context< CSynchronizationAction >().addRequest( request );

		if ( !allowed )
			context< CSynchronizationAction >().setExit();
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
								, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

				context< CSynchronizationAction >().setRequestKey( _messageResult.m_message.m_header.m_id );

				return transit< CSynchronizedProvideCopy >();
			}
			else if ( infoRequest.m_kind == common::CInfoKind::BitcoinHeaderAsk )
			{
				context< CSynchronizationAction >().addRequest(
							new common::CAckRequest(
								context< CSynchronizationAction >().getActionKey()
								, _messageResult.m_message.m_header.m_id
								, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

				CAutoFile file(OpenHeadFile(true), SER_DISK, CLIENT_VERSION);
				CBlockHeader header;
				file >> header;

				common::CSendMessageRequest * request =
						new common::CSendMessageRequest(
							common::CPayloadKind::SynchronizationBitcoinHeader
							, context< CSynchronizationAction >().getActionKey()
							, _messageResult.m_message.m_header.m_id
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) );

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
							, new CByKeyMediumFilter( context< CSynchronizationAction >().getPartnerKey() ) );

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
					, new CByKeyMediumFilter( context< CSynchronizationAction >().getPartnerKey() ) );

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
					, new CByKeyMediumFilter( context< CSynchronizationAction >().getPartnerKey() ) );

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
							, new CByKeyMediumFilter( _messageResult.m_pubKey ) ) );

			if ( synchronizationGet.m_number < m_storedBlocks && synchronizationGet.m_kind == common::CBlockKind::Segment )
			{
				setBlock( synchronizationGet.m_number );
				m_exit = synchronizationGet.m_number == m_storedBlocks - 1;

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
		if ( m_exit )
		{
			context< CSynchronizationAction >().setExit();

			common::CSendMessageRequest * request =
					new common::CSendMessageRequest(
						common::CPayloadKind::FullRankingInfo
						, context< CSynchronizationAction >().getActionKey()
						, m_id
						, new CMediumClassFilter( common::CMediumKinds::DimsNodes ) );

			request->addPayload(
						common::CRankingFullInfo(
							CReputationTracker::getInstance()->getAllyTrackers()
							, CReputationTracker::getInstance()->getAllyMonitors()
							, CReputationTracker::getInstance()->getTrackers()
							, CReputationControlAction::getInstance()->getActionKey() ) );

			context< CSynchronizationAction >().addRequest( request );
		}

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
	bool m_exit;
};

CSynchronizationAction::CSynchronizationAction( CPubKey const & _partnerKey )
	: m_partnerKey( _partnerKey )
{
	initiate();
	process_event( CSwitchToSynchronizing() );
}

CSynchronizationAction::CSynchronizationAction( uint256 const & _id, uint256 const & _actionKey, CPubKey const & _partnerKey )
	: common::CScheduleAbleAction( _actionKey )
	, m_requestKey( _id )
	, m_partnerKey( _partnerKey )
{
	initiate();
	process_event( CSwitchToSynchronized() );
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

bool
CSynchronizationAction::isRequestInitialized() const
{
	return !m_requests.empty();
}

}

//struct CDiskBlock;
