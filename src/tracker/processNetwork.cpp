// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/communicationProtocol.h"
#include "common/actionHandler.h"

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
		if (
				message.m_header.m_payloadKind == common::CPayloadKind::Transactions
			|| message.m_header.m_payloadKind == common::CPayloadKind::StatusTransactions
			)
		{
			CPubKey pubKey;
			if( !CTrackerNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) )
			{
				assert( !"for now assert this" );
				return true;
			}

			common::CNodeMedium< common::CTrackerBaseMedium > * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_id, common::CMessageResult( message, convertToInt( nodeMedium->getNode() ), pubKey ) );
			}
			else
			{
				CValidateTransactionsAction * validateTransactionsAction= new CValidateTransactionsAction( message.m_header.m_actionKey );
				validateTransactionsAction->process_event( common::CMessageResult( message, convertToInt( nodeMedium->getNode() ), pubKey ) );
				common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( validateTransactionsAction );
			}
		}
		else if (
					  message.m_header.m_payloadKind == common::CPayloadKind::ConnectCondition
				|| message.m_header.m_payloadKind == common::CPayloadKind::InfoReq
				|| message.m_header.m_payloadKind == common::CPayloadKind::Result
				|| message.m_header.m_payloadKind == common::CPayloadKind::Balance
				|| message.m_header.m_payloadKind == common::CPayloadKind::RoleInfo
				|| message.m_header.m_payloadKind == common::CPayloadKind::NetworkInfo
				|| message.m_header.m_payloadKind == common::CPayloadKind::ValidRegistration
				|| message.m_header.m_payloadKind == common::CPayloadKind::SynchronizationInfo
				|| message.m_header.m_payloadKind == common::CPayloadKind::SynchronizationBitcoinHeader
				|| message.m_header.m_payloadKind == common::CPayloadKind::SynchronizationHeader
				 || message.m_header.m_payloadKind == common::CPayloadKind::SynchronizationBlock
				 || message.m_header.m_payloadKind == common::CPayloadKind::ExtendRegistration
				 )
		{
			common::CNodeMedium< common::CTrackerBaseMedium > * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			CPubKey pubKey;
			if( !CTrackerNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) )
			{
				assert( !"for now assert this" );
				return true;
			}

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				if ( message.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
					nodeMedium->addActionResponse( message.m_header.m_actionKey, common::CMessageResult( message, convertToInt( nodeMedium->getNode() ), pubKey ) );
				else
					nodeMedium->setResponse( message.m_header.m_id, common::CMessageResult( message, convertToInt( nodeMedium->getNode() ), pubKey ) );
			}
			else if ( message.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
			{
				CProvideInfoAction * provideInfoAction= new CProvideInfoAction(
							  message.m_header.m_actionKey
							, convertToInt( nodeMedium->getNode() )
							);

				provideInfoAction->process_event( common::CMessageResult( message, convertToInt( nodeMedium->getNode() ), pubKey ) );

				common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( provideInfoAction );
			}
			else if ( message.m_header.m_payloadKind == common::CPayloadKind::ExtendRegistration )
			{
				CRegisterAction * registerAction
						= new CRegisterAction( message.m_header.m_actionKey, convertToInt( pfrom ) );

				registerAction->process_event( common::CMessageResult( message, convertToInt( nodeMedium->getNode() ), pubKey ) );

				common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( registerAction );

			}
		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::IntroductionReq )
		{
			common::CIdentifyMessage identifyMessage;
			convertPayload( message, identifyMessage );

			common::CNodeMedium< common::CTrackerBaseMedium > * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

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

				common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( connectNodeAction );
			}

		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::Ack )
		{
			common::CAck ack;

			common::convertPayload( message, ack );

			common::CNodeMedium< common::CTrackerBaseMedium > * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_id, common::CAckResult( convertToInt( nodeMedium->getNode() ) ) );
			}
			else
			{
				assert(!"it should be existing action");

			}
		}
		else if (
					  message.m_header.m_payloadKind == common::CPayloadKind::Ping
				|| message.m_header.m_payloadKind == common::CPayloadKind::Pong )
		{
			common::CNodeMedium< common::CTrackerBaseMedium > * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				bool isPing = message.m_header.m_payloadKind == common::CPayloadKind::Ping;
				nodeMedium->setResponse( message.m_header.m_id, common::CPingPongResult( isPing, convertToInt( nodeMedium->getNode() ) ) );
			}
			else
			{
				if ( message.m_header.m_payloadKind == common::CPayloadKind::Ping )
				{

					CPingAction * pingAction
							= new CPingAction( message.m_header.m_actionKey, convertToInt( pfrom ) );

					pingAction->process_event( common::CStartPongEvent() );

					common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( pingAction );
				}
				else
				{
					assert(!"it should be existing action");
				}
			}
		}
		//NetworkInfo
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
	common::CNodesManager< CTrackerTypes >::getInstance()->eraseMedium( convertToInt( this ) );
}
}
