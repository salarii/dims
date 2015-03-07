// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "processNetwork.h"

#include "common/nodesManager.h"
#include "common/communicationProtocol.h"
#include "common/actionHandler.h"

#include "common/nodeMedium.h"

#include "configureMonitorActionHandler.h"
#include "monitor/connectNodeAction.h"
#include "monitor/reputationTracer.h"
#include "monitor/monitorNodeMedium.h"

namespace monitor
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
	if ( !CReputationTracker::getInstance()->getMediumForNode( pfrom ) )
	{
		CReputationTracker::getInstance()->addNode( new CMonitorNodeMedium( pfrom ) );
	}

	BOOST_FOREACH( common::CMessage const & message, messages )
	{


		if (
				   message.m_header.m_payloadKind == common::CPayloadKind::RoleInfo
				|| message.m_header.m_payloadKind == common::CPayloadKind::Result
				|| message.m_header.m_payloadKind == common::CPayloadKind::NetworkInfo
				|| message.m_header.m_payloadKind == common::CPayloadKind::InfoRes
				)
		{
			common::CNodeMedium< common::CMonitorBaseMedium > * nodeMedium = CReputationTracker::getInstance()->getMediumForNode( pfrom );
			// not necessarily have to pass this
			CPubKey key;
			CReputationTracker::getInstance()->getNodeToKey( convertToInt( nodeMedium->getNode() ), key );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_actionKey, common::CMessageResult( message, convertToInt( nodeMedium->getNode() ), key ) );
			}
			else
			{
			}
		}
		/*	{
		CPubKey pubKey;
			if( !CTrackerNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) )
			{
				assert( !"for now assert this" );
				return true;
			}
		}*/
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
		{
			//
		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::IntroductionReq )
		{
			common::CIdentifyMessage identifyMessage;
			convertPayload( message, identifyMessage );

			common::CNodeMedium< common::CMonitorBaseMedium > * nodeMedium = CReputationTracker::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_actionKey, common::CIdentificationResult( identifyMessage.m_payload, identifyMessage.m_signed, identifyMessage.m_key, pfrom->addr  ) );
			}
			else
			{
				CConnectNodeAction * connectNodeAction= new CConnectNodeAction(
							  nodeMedium->getNode()->addr
							, message.m_header.m_actionKey
							, convertToInt( nodeMedium->getNode() ) );

				connectNodeAction->process_event( common::CIdentificationResult( identifyMessage.m_payload, identifyMessage.m_signed, identifyMessage.m_key, pfrom->addr ) );

				common::CActionHandler< common::CMonitorTypes >::getInstance()->executeAction( connectNodeAction );
			}

		}
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::Ack )
		{
			common::CAck ack;

			common::convertPayload( message, ack );

			common::CNodeMedium< common::CMonitorBaseMedium > * nodeMedium = CReputationTracker::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_actionKey, common::CAckResult( convertToInt( nodeMedium->getNode() ) ) );
			}
		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::Uninitiated )
		{
			//
		}
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

