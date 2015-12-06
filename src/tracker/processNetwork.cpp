// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/communicationProtocol.h"
#include "common/actionHandler.h"
#include "common/networkActionRegister.h"
#include "common/events.h"

#include "tracker/processNetwork.h"
#include "tracker/trackerNodesManager.h"
#include "tracker/filters.h"
#include "tracker/trackerNodeMedium.h"
#include "tracker/connectNodeAction.h"
#include "tracker/synchronizationAction.h"
#include "tracker/validateTransactionsAction.h"
#include "tracker/provideInfoAction.h"
#include "tracker/pingAction.h"
#include "tracker/registerAction.h"
#include "tracker/passTransactionAction.h"
#include "tracker/updateNetworkDataAction.h"
#include "tracker/activityControllerAction.h"

namespace tracker
{

CProcessNetwork * CProcessNetwork::ms_instance = NULL;

CProcessNetwork*
CProcessNetwork::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CProcessNetwork();
	};
	return ms_instance;
}

bool
CProcessNetwork::processMessage(common::CSelfNode* pfrom, CDataStream& vRecv)
{
	std::vector< common::CMessage > messages;
	vRecv >> messages;

// it is  stupid  to call this over and over again
	if ( !CTrackerNodesManager::getInstance()->getMediumForNode( pfrom ) )
	{
		CTrackerNodesManager::getInstance()->addNode( new CTrackerNodeMedium( pfrom ) );
	}

	BOOST_FOREACH( common::CMessage const & message, messages )
	{
		if ( message.m_header.m_payloadKind == common::CPayloadKind::IntroductionReq )
		{
			common::CIdentifyMessage identifyMessage;
			convertPayload( message, identifyMessage );

			common::CNodeMedium * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->addActionResponse( message.m_header.m_actionKey, common::CIdentificationResult( identifyMessage.m_payload, identifyMessage.m_signed, identifyMessage.m_key, pfrom->addr, message.m_header.m_id ) );
			}
			else
			{
				CConnectNodeAction * connectNodeAction= new CConnectNodeAction(
							  message.m_header.m_actionKey
							, convertToInt( nodeMedium->getNode() ) );

				connectNodeAction->process_event( common::CIdentificationResult( identifyMessage.m_payload, identifyMessage.m_signed, identifyMessage.m_key, pfrom->addr, message.m_header.m_id ) );

				common::CActionHandler::getInstance()->executeAction( connectNodeAction );
			}

		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::Ack )
		{
			common::CAck ack;

			common::convertPayload( message, ack );

			common::CNodeMedium * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_id, common::CAckResult( convertToInt( nodeMedium->getNode() ) ) );
			}
		}
		else
		{
			common::CNodeMedium * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			CPubKey pubKey;
			if( !CTrackerNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) )
			{
				return true;
			}

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				if ( message.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
					nodeMedium->addActionResponse( message.m_header.m_actionKey, common::CMessageResult( message, pubKey ) );
				else
					nodeMedium->setResponse( message.m_header.m_id, common::CMessageResult( message, pubKey ) );
			}
			else if ( message.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
			{
				CProvideInfoAction * provideInfoAction= new CProvideInfoAction(
							  message.m_header.m_actionKey
							, pubKey
							);

				provideInfoAction->process_event( common::CMessageResult( message, pubKey ) );

				common::CActionHandler::getInstance()->executeAction( provideInfoAction );
			}
			else if ( message.m_header.m_payloadKind == common::CPayloadKind::ExtendRegistration )
			{
				CRegisterAction * registerAction
						= new CRegisterAction( message.m_header.m_actionKey, pubKey );

				registerAction->process_event( common::CMessageResult( message, pubKey ) );

				common::CActionHandler::getInstance()->executeAction( registerAction );

			}
			else if ( message.m_header.m_payloadKind == common::CPayloadKind::ClientTransaction )
			{
				CPassTransactionAction * passTransactionAction
						= new CPassTransactionAction( message.m_header.m_actionKey );

				passTransactionAction->process_event( common::CMessageResult( message, pubKey ) );

				common::CActionHandler::getInstance()->executeAction( passTransactionAction );
			}
			else if ( message.m_header.m_payloadKind == common::CPayloadKind::Transactions )
			{
				CValidateTransactionsAction * validateTransactionsAction= new CValidateTransactionsAction( message.m_header.m_actionKey );

				validateTransactionsAction->process_event( common::CMessageResult( message, pubKey ) );

				common::CActionHandler::getInstance()->executeAction( validateTransactionsAction );
			}
			else if ( message.m_header.m_payloadKind == common::CPayloadKind::FullRankingInfo )
			{
				CUpdateNetworkDataAction * updateNetworkDataAction = new CUpdateNetworkDataAction( message.m_header.m_actionKey );

				updateNetworkDataAction->process_event( common::CMessageResult( message, pubKey ) );

				common::CActionHandler::getInstance()->executeAction( updateNetworkDataAction );
			}
			else if ( message.m_header.m_payloadKind == common::CPayloadKind::ActivationStatus )
			{

				CActivityControllerAction * activityControllerAction = new CActivityControllerAction( message.m_header.m_actionKey );

				activityControllerAction->process_event( common::CMessageResult( message, pubKey ) );

				common::CActionHandler::getInstance()->executeAction( activityControllerAction );
			}
			else if ( message.m_header.m_payloadKind == common::CPayloadKind::Ping )
			{
				CPingAction * pingAction = new CPingAction( message.m_header.m_actionKey );

				pingAction->process_event( common::CMessageResult( message, pubKey ) );

				common::CActionHandler::getInstance()->executeAction( pingAction );
			}
		}
	}
	return true;
}


bool
CProcessNetwork::sendMessages(common::CSelfNode* pto, bool fSendTrickle)
{
	pto->sendMessages();

	return true;
}

}

namespace common
{

void
CSelfNode::clearManager()
{
	common::CNodesManager::getInstance()->eraseMedium( convertToInt( this ) );
}

}
