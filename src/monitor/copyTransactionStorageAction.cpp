// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/setResponseVisitor.h"
#include "common/commonRequests.h"
#include "common/commonEvents.h"
#include "common/mediumKinds.h"
#include "common/originAddressScanner.h"

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
						context< CopyTransactionStorageAction >().getActionKey()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

		context< CCopyTransactionStorageAction >().addRequest(
					new common::CStorageInfoRequest< common::CMonitorTypes >(
						  CCopyStorageHandler::getInstance()->getTimeStamp()
						, CCopyStorageHandler::getInstance()->getDiscBlockSize()
						, _messageResult.m_message.m_header.m_id
						, new CSpecificMediumFilter( _messageResult.m_nodeIndicator ) ) );

		CCopyStorageHandler::getInstance()->getSegmentHeaderSize();

		return discard_event();
	}
	typedef boost::mpl::list<
	boost::statechart::custom_reaction< common::CMessageResult >
	> reactions;
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
