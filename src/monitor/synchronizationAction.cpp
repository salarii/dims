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
#include "monitor/copyStorageHandler.h"

namespace monitor
{

unsigned const SynchronisingGetInfoTime = 10000;//milisec

unsigned const SynchronisingWaitTime = 15000;

struct CSynchronizingGetInfo;
struct CSynchronizingHeaders;
struct CSynchronizedUninitialized;
struct CSynchronizedProvideCopy;

struct CSwitchToSynchronizing : boost::statechart::event< CSwitchToSynchronizing >
{
};

struct CUninitiated : boost::statechart::simple_state< CUninitiated, CSynchronizationAction >
{
	typedef boost::mpl::list<
	boost::statechart::transition< CSwitchToSynchronizing, CSynchronizingGetInfo >,
	boost::statechart::transition< CSwitchToSynchronized, CSynchronizedUninitialized >
	> reactions;
};

struct CSynchronizingGetInfo : boost::statechart::state< CSynchronizingGetInfo, CSynchronizationAction >
{
	CSynchronizingGetInfo( my_context ctx ) : my_base( ctx )
	{
	}
};

struct CSynchronized;

struct CSynchronizingBlocks : boost::statechart::state< CSynchronizingBlocks, CSynchronizationAction >
{
	CSynchronizingBlocks( my_context ctx ) : my_base( ctx )
	{
	}
};


struct CSynchronizingHeaders : boost::statechart::state< CSynchronizingHeaders, CSynchronizationAction >
{
	CSynchronizingHeaders( my_context ctx ) : my_base( ctx )
	{
	}
};

struct CSynchronizedUninitialized : boost::statechart::state< CSynchronizedUninitialized, CSynchronizationAction >
{
	CSynchronizedUninitialized( my_context ctx ) : my_base( ctx )
	{
		context< CSynchronizationAction >().addRequest(
					new common::CTimeEventRequest< common::CMonitorTypes >(
						SynchronisingWaitTime
						, new CMediumClassFilter( common::CMediumKinds::Time ) ) );

		context< CSynchronizationAction >().addRequest(
					new common::CAckRequest< common::CMonitorTypes >(
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
							new common::CAckRequest< common::CMonitorTypes >(
								context< CSynchronizationAction >().getActionKey()
								, _messageResult.m_message.m_header.m_id
								, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

				context< CSynchronizationAction >().setRequestKey( _messageResult.m_message.m_header.m_id );

				return transit< CSynchronizedProvideCopy >();
			}
			else if ( infoRequest.m_kind == common::CInfoKind::BitcoinHeaderAsk )
			{
				context< CSynchronizationAction >().addRequest(
							new common::CAckRequest< common::CMonitorTypes >(
								context< CSynchronizationAction >().getActionKey()
								, _messageResult.m_message.m_header.m_id
								, new CSpecificMediumFilter( context< CSynchronizationAction >().getNodeIdentifier() ) ) );

				CAutoFile file(OpenHeadFile(true), SER_DISK, CLIENT_VERSION);
				CBlockHeader header;
				file >> header;

				common::CSendMessageRequest< common::CMonitorTypes > * request =
						new common::CSendMessageRequest< common::CMonitorTypes >(
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
					new common::CTimeEventRequest< common::CMonitorTypes >(
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
				common::CSendMessageRequest< common::CMonitorTypes > * request =
						new common::CSendMessageRequest< common::CMonitorTypes >(
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
					new common::CTimeEventRequest< common::CMonitorTypes >(
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

		common::CSendMessageRequest< common::CMonitorTypes > * request =
				new common::CSendMessageRequest< common::CMonitorTypes >(
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

		common::CSendMessageRequest< common::CMonitorTypes > * request =
				new common::CSendMessageRequest< common::CMonitorTypes >(
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
						new common::CAckRequest< common::CMonitorTypes >(
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
	: common::CScheduleAbleAction< common::CMonitorTypes >( _actionKey )
	, m_requestKey( _id )
	, m_nodeIdentifier( _nodeIndicator )
{
	initiate();
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
