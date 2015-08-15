// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/commonRequests.h"
#include "common/commonEvents.h"
#include "common/mediumKinds.h"
#include "common/originAddressScanner.h"
#include "common/supportTransactionsDatabase.h"

#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/custom_reaction.hpp>

#include "monitor/monitorController.h"
#include "monitor/filters.h"
#include "monitor/monitorNodeMedium.h"
#include "monitor/copyTransactionStorageAction.h"
#include "monitor/copyStorageHandler.h"

namespace monitor
{
struct CSynchronizingBlocks;

// send  info  about  copy if  it  exist  and  what  size  it  is
// request  for  downloading
// provide fragments
//  provide  missing transactions
// flag operating and ready

struct CProvideInfoAboutStorage : boost::statechart::state< CProvideInfoAboutStorage, CCopyTransactionStorageAction >
{
	CProvideInfoAboutStorage( my_context ctx ) : my_base( ctx )
	{}

	boost::statechart::result react( common::CMessageResult const & _messageResult )
	{
		common::CMessage orginalMessage;
		if ( !common::CommunicationProtocol::unwindMessage( _messageResult.m_message, orginalMessage, GetTime(), _messageResult.m_pubKey ) )
			assert( !"service it somehow" );

		common::CRegistrationTerms connectCondition;

		common::convertPayload( orginalMessage, connectCondition );

		context< CCopyTransactionStorageAction >().addRequest(
					new common::CAckRequest< common::CMonitorTypes >(
						context< CCopyTransactionStorageAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );



		CCopyStorageHandler::getInstance()->getSegmentHeaderSize();

		return discard_event();
	}

	typedef boost::mpl::list<
	boost::statechart::transition< common::CAckEvent, CSynchronizingBlocks >,
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
};

struct CSynchronizingBlocks : boost::statechart::state< CSynchronizingBlocks, CCopyTransactionStorageAction >
{
	CSynchronizingBlocks( my_context ctx ) : my_base( ctx )
	{

	}


	boost::statechart::result react( common::CEndEvent const & )
	{

		return discard_event();
	}

	~CSynchronizingBlocks()
	{
		common::CSegmentFileStorage::getInstance()->releaseSynchronizationInProgress();
	}

};


struct CSynchronizingHeaders : boost::statechart::state< CSynchronizingHeaders, CCopyTransactionStorageAction >
{

};

struct CSynchronized : boost::statechart::state< CSynchronized, CCopyTransactionStorageAction >
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

		context< CCopyTransactionStorageAction >().dropRequests();
		context< CCopyTransactionStorageAction >().addRequest( new common::CSetNextBlockRequest< common::CDiskBlock, common::CMonitorTypes >(
															context< CCopyTransactionStorageAction >().getActionKey()
															, new CSpecificMediumFilter( context< CCopyTransactionStorageAction >().getNodeIdentifier() )
															, m_diskBlock
															, m_currentBlock++) );
	}


	void setHeaders()
	{
		common::CSegmentFileStorage::getInstance()->getSegmentHeader( m_currentHeader, *m_segmentHeader );

		context< CCopyTransactionStorageAction >().dropRequests();
		context< CCopyTransactionStorageAction >().addRequest( new common::CSetNextBlockRequest< common::CSegmentHeader, common::CMonitorTypes >(
															context< CCopyTransactionStorageAction >().getActionKey()
															, new CSpecificMediumFilter( context< CCopyTransactionStorageAction >().getNodeIdentifier() )
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
			context< CCopyTransactionStorageAction >().dropRequests();
		}

		return discard_event();
	}

	boost::statechart::result react( common::CAckEvent const & )
	{
		context< CCopyTransactionStorageAction >().dropRequests();

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

struct CWaitForStorageCopy : boost::statechart::state< CWaitForStorageCopy, CCopyTransactionStorageAction >
{
	CWaitForStorageCopy( my_context ctx ) : my_base( ctx )
	{}

	typedef boost::mpl::list<
	> reactions;
};

struct CSendStorage : boost::statechart::state< CSendStorage, CCopyTransactionStorageAction >
{
	CSendStorage( my_context ctx ) : my_base( ctx )
	{}

	typedef boost::mpl::list<
	> reactions;
};

struct CSendRecentTransactions
{
	//indicate start
};

CCopyTransactionStorageAction::CCopyTransactionStorageAction()
{}

void
CCopyTransactionStorageAction::accept( common::CSetResponseVisitor< common::CMonitorTypes > & _visitor )
{
	_visitor.visit( *this );
}

}
