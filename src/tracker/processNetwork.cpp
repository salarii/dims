// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "processNetwork.h"
#include "trackerNodesManager.h"
#include "common/communicationProtocol.h"
#include "common/actionHandler.h"
#include "trackerFilters.h"

#include "trackerNodeMedium.h"
#include "connectNodeAction.h"
#include "synchronizationAction.h"

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
		CTrackerNodesManager::getInstance()->addNode( pfrom );
	}

	BOOST_FOREACH( common::CMessage const & message, messages )
	{


		if ( message.m_header.m_payloadKind == common::CPayloadKind::Transactions )
		{
			//
		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
		{
			//
		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::IntroductionReq )
		{
			common::CIdentifyMessage identifyMessage;
			convertPayload( message, identifyMessage );

			CTrackerNodeMedium * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( identifyMessage.m_actionKey ) )
			{
				nodeMedium->setResponse( identifyMessage.m_actionKey, common::CIdentificationResult( identifyMessage.m_payload, identifyMessage.m_signed, identifyMessage.m_key ) );
			}
			else
			{
				CConnectNodeAction * connectTrackerAction= new CConnectNodeAction(
							  identifyMessage.m_actionKey
							, identifyMessage.m_payload
							, convertToInt( nodeMedium->getNode() ) );

				common::CActionHandler< TrackerResponses >::getInstance()->executeAction( connectTrackerAction );

			}

		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::SynchronizationInfo )
		{
			common::CSynchronizationInfo synchronizationInfo;
			convertPayload( message, synchronizationInfo );

			CTrackerNodeMedium * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( synchronizationInfo.m_actionKey ) )
			{
				nodeMedium->setResponse( synchronizationInfo.m_actionKey, CSynchronizationInfoResult( synchronizationInfo.m_timeStamp ) );
			}
			else
			{
				common::CActionHandler< TrackerResponses >::getInstance()->executeAction( new CSynchronizationAction() );
			}
		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::Uninitiated )
		{
			CPubKey pubKey;
			if( !CTrackerNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) );

			common::CMessage orginalMessage;
			if ( !common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey ) )
				assert( !"service it somehow" );

			common::CSynchronizationInfo synchronizationInfo;

			common::convertPayload( orginalMessage, synchronizationInfo );

			CTrackerNodeMedium * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( synchronizationInfo.m_actionKey ) )
			{
				nodeMedium->setResponse( synchronizationInfo.m_actionKey, CSynchronizationInfoResult( synchronizationInfo.m_timeStamp ) );
			}
			else
			{
				/*CConnectNodeAction * connectTrackerAction= new CConnectNodeAction( synchronizationInfo.m_actionKey, identifyMessage.m_payload, convertToInt( nodeMedium->getNode() ) );
				common::CActionHandler< TrackerResponses >::getInstance()->executeAction( connectTrackerAction );*/
			}
		}
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::RoleInfo )
		{
			CPubKey pubKey;
			if( !CTrackerNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) );

			common::CMessage orginalMessage;
			if ( !common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey ) )
				assert( !"service it somehow" );

			common::CNetworkRole networkRole;

			common::convertPayload( orginalMessage, networkRole );

			CTrackerNodeMedium * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( networkRole.m_actionKey ) )
			{
				nodeMedium->setResponse( networkRole.m_actionKey, common::CRoleResult( networkRole.m_role ) );
			}
			else
			{
				assert(!"it should be existing action");

			}
		}
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::NetworkInfo )
		{
			CPubKey pubKey;
			if( CTrackerNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) );

			common::CMessage orginalMessage;
			if ( !common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey ) )
				assert( !"service it somehow" );

			common::CKnownNetworkInfo knownNetworkInfo;

			common::convertPayload( orginalMessage, knownNetworkInfo );

			CTrackerNodeMedium * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( knownNetworkInfo.m_actionKey ) )
			{
				nodeMedium->setResponse( knownNetworkInfo.m_actionKey, common::CNetworkInfoResult( knownNetworkInfo.m_networkInfo ) );
			}
			else
			{
				assert(!"it should be existing action");

			}
		}
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::Ack )
		{
			CPubKey pubKey;
			if ( !CTrackerNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) )
				;

			common::CMessage orginalMessage;
			if ( !common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey ) )
				assert( !"service it somehow" );

			common::CAck ack;

			common::convertPayload( orginalMessage, ack );

			CTrackerNodeMedium * nodeMedium = CTrackerNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( ack.m_actionKey ) )
			{
				nodeMedium->setResponse( ack.m_actionKey, common::CAckResult() );
			}
			else
			{
				assert(!"it should be existing action");

			}
		}
		//NetworkInfo
	}
	/*


*/
	return true;
}


bool
CProcessNetwork::sendMessages(common::CSelfNode* pto, bool fSendTrickle)
{
	pto->sendMessages();

	return true;
}

}
