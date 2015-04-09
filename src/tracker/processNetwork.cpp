// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/communicationProtocol.h"
#include "common/actionHandler.h"

#include "tracker/processNetwork.h"
#include "tracker/trackerNodesManager.h"
#include "tracker/trackerFilters.h"
#include "tracker/trackerNodeMedium.h"
#include "tracker/connectNodeAction.h"
#include "tracker/synchronizationAction.h"
#include "tracker/validateTransactionsAction.h"
#include "tracker/provideInfoAction.h"
#include "tracker/pingAction.h"

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
				|| message.m_header.m_payloadKind == common::CPayloadKind::RoleInfo
				|| message.m_header.m_payloadKind == common::CPayloadKind::NetworkInfo
				|| message.m_header.m_payloadKind == common::CPayloadKind::InfoReq
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
		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::IntroductionReq )
		{
			common::CIdentifyMessage identifyMessage;
			convertPayload( message, identifyMessage );

			common::CNodeMedium< common::CTrackerBaseMedium > * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_id, common::CIdentificationResult( identifyMessage.m_payload, identifyMessage.m_signed, identifyMessage.m_key, pfrom->addr ) );
			}
			else
			{
				CConnectNodeAction * connectNodeAction= new CConnectNodeAction(
							  message.m_header.m_actionKey
							, convertToInt( nodeMedium->getNode() ) );

				connectNodeAction->process_event( common::CIdentificationResult( identifyMessage.m_payload, identifyMessage.m_signed, identifyMessage.m_key, pfrom->addr ) );

				common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction( connectNodeAction );
			}

		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::SynchronizationInfo )
		{
			CPubKey pubKey;
			if( !CTrackerNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) );

			common::CMessage orginalMessage;
			if ( !common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey ) )
				assert( !"service it somehow" );

			common::CSynchronizationInfo synchronizationInfo;
			convertPayload( message, synchronizationInfo );

			common::CNodeMedium< common::CTrackerBaseMedium > * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_id, CSynchronizationInfoResult( synchronizationInfo.m_timeStamp, convertToInt( nodeMedium->getNode() ), message.m_header.m_id ) );
			}
			else
			{
				common::CActionHandler< common::CTrackerTypes >::getInstance()->executeAction(
							new CSynchronizationAction( message.m_header.m_actionKey, convertToInt( nodeMedium->getNode() ), synchronizationInfo.m_timeStamp ) );
			}
		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::Uninitiated )
		{

		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::Get )
		{
			CPubKey pubKey;
			if( !CTrackerNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) );

			common::CMessage orginalMessage;
			if ( !common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey ) )
				assert( !"service it somehow" );

			common::CGet get;
			common::convertPayload( orginalMessage, get );

			common::CNodeMedium< common::CTrackerBaseMedium > * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_id, common::CGetPrompt( get.m_type ) );
			}
			else
			{
				assert(!"it should be existing action");
			}
		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::SynchronizationBlock )
		{
			CPubKey pubKey;
			if( !CTrackerNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) );

			common::CMessage orginalMessage;
			if ( !common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey ) )
				assert( !"service it somehow" );

			CSynchronizationBlock synchronizationBlock( new CDiskBlock, -1 );
			common::convertPayload( orginalMessage, synchronizationBlock );

			common::CNodeMedium< common::CTrackerBaseMedium > * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_id, CSynchronizationBlockResult< CDiskBlock >( synchronizationBlock.m_diskBlock, synchronizationBlock.m_blockIndex ) );
			}
			else
			{
				assert(!"it should be existing action");
			}
		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::SynchronizationHeader )
		{
			CPubKey pubKey;
			if( !CTrackerNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) );

			common::CMessage orginalMessage;
			if ( !common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey ) )
				assert( !"service it somehow" );

			CSynchronizationSegmentHeader synchronizationSegmentHeader( new CSegmentHeader, -1 );
			common::convertPayload( orginalMessage, synchronizationSegmentHeader );

			common::CNodeMedium< common::CTrackerBaseMedium > * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_id, CSynchronizationBlockResult< CSegmentHeader >( synchronizationSegmentHeader.m_segmentHeader, synchronizationSegmentHeader.m_blockIndex ) );
			}
			else
			{
				assert(!"it should be existing action");

			}
		}
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::Ack )
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
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::End )
		{
			common::CEnd end;

			common::convertPayload( message, end );

			common::CNodeMedium< common::CTrackerBaseMedium > * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_id, common::CEndEvent() );
			}
			else
			{
				assert(!"it should be existing action");

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
