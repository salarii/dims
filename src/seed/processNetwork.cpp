// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "processNetwork.h"
#include "seedNodeMedium.h"
#include "seedNodesManager.h"
#include "acceptNodeAction.h"
#include "common/actionHandler.h"
#include "common/communicationProtocol.h"
#include "common/commonEvents.h"

namespace seed
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
	if ( !CSeedNodesManager::getInstance()->getMediumForNode( pfrom ) )
	{
		CSeedNodesManager::getInstance()->addNode( new CSeedNodeMedium( pfrom ) );
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

			common::CNodeMedium< SeedResponses > * nodeMedium = CSeedNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_actionKey, common::CIdentificationResult( identifyMessage.m_payload, identifyMessage.m_signed, identifyMessage.m_key, pfrom->addr ) );
			}
			else
			{
				CAcceptNodeAction * connectTrackerAction= new CAcceptNodeAction( message.m_header.m_actionKey, identifyMessage.m_payload, convertToInt( nodeMedium->getNode() ) );
				common::CActionHandler< SeedResponses >::getInstance()->executeAction( connectTrackerAction );
			}

		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::Uninitiated )
		{
			//
		}
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::RoleInfo )
		{
			CPubKey pubKey;
			if ( !CSeedNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) )
				;//service  error  somehow, can't  decode  action  at  this point  so it  have  to  be  done as  common  solution  for  all  such  issues
			common::CMessage orginalMessage;
			if ( !common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey ) )
				assert( !"service it somehow" );

			common::CNetworkRole networkRole;

			common::convertPayload( orginalMessage, networkRole );

			common::CNodeMedium< SeedResponses > * nodeMedium = CSeedNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_actionKey, common::CRoleResult( networkRole.m_role ) );
			}
		}
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::NetworkInfo )
		{
			CPubKey pubKey;
			if ( !CSeedNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) )
				;

			common::CMessage orginalMessage;
			if ( !common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey ) )
				assert( !"service it somehow" );

			common::CKnownNetworkInfo knownNetworkInfo;

			common::convertPayload( orginalMessage, knownNetworkInfo );

			common::CNodeMedium< SeedResponses > * nodeMedium = CSeedNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_actionKey, common::CNetworkInfoResult( knownNetworkInfo.m_trackersInfo, knownNetworkInfo.m_monitorsInfo ) );
			}
		}
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::Ack )
		{
			common::CAck ack;

			common::convertPayload( message, ack );

			common::CNodeMedium< SeedResponses > * nodeMedium = CSeedNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_actionKey, common::CAckResult( convertToInt( nodeMedium->getNode() ) ) );
			}
		}
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::End )
		{
			common::CEnd end;

			common::convertPayload( message, end );

			common::CNodeMedium< SeedResponses > * nodeMedium = CSeedNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_actionKey, common::CEndEvent() );
			}
		}
	}
}

bool
CProcessNetwork::sendMessages(common::CSelfNode* pto, bool fSendTrickle)
{
	pto->sendMessages();

	return true;
}

}
