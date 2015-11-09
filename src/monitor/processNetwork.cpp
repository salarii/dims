// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/nodesManager.h"
#include "common/communicationProtocol.h"
#include "common/actionHandler.h"
#include "common/nodeMedium.h"
#include "common/networkActionRegister.h"
#include "common/events.h"

#include "monitor/processNetwork.h"
#include "monitor/connectNodeAction.h"
#include "monitor/reputationTracer.h"
#include "monitor/monitorNodeMedium.h"
#include "monitor/admitTrackerAction.h"
#include "monitor/admitTransactionsBundle.h"
#include "monitor/pingAction.h"
#include "monitor/provideInfoAction.h"
#include "monitor/synchronizationAction.h"
#include "monitor/admitTransactionsBundle.h"
#include "monitor/enterNetworkAction.h"
#include "monitor/updateNetworkDataAction.h"
#include "monitor/activityControllerAction.h"

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

		if ( message.m_header.m_payloadKind == common::CPayloadKind::IntroductionReq )
		{
			common::CIdentifyMessage identifyMessage;
			convertPayload( message, identifyMessage );

			common::CNodeMedium * nodeMedium = CReputationTracker::getInstance()->getMediumForNode( pfrom );

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
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::Ack )
		{
			common::CAck ack;

			common::convertPayload( message, ack );

			common::CNodeMedium * nodeMedium = CReputationTracker::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_id, common::CAckResult( convertToInt( nodeMedium->getNode() ) ) );
			}
		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::Uninitiated )
		{
			//
		}
		else
		{
			common::CNodeMedium * nodeMedium = CReputationTracker::getInstance()->getMediumForNode( pfrom );
			// not necessarily have to pass this
			CPubKey key;
			if ( !CReputationTracker::getInstance()->getPublicKey( pfrom->addr, key ) )
				assert( !"this  can't fail" );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				if (
							  message.m_header.m_payloadKind == common::CPayloadKind::InfoReq
						|| message.m_header.m_payloadKind == common::CPayloadKind::AdmitProof
						|| message.m_header.m_payloadKind == common::CPayloadKind::AdmitAsk
						|| message.m_header.m_payloadKind == common::CPayloadKind::SynchronizationInfo
						|| message.m_header.m_payloadKind == common::CPayloadKind::SynchronizationGet
						|| message.m_header.m_payloadKind == common::CPayloadKind::Transactions
					)
					nodeMedium->addActionResponse( message.m_header.m_actionKey, common::CMessageResult( message, key ) );
				else
					nodeMedium->setResponse( message.m_header.m_id, common::CMessageResult( message, key ) );
			}
			else
			{
				if ( message.m_header.m_payloadKind == common::CPayloadKind::AdmitAsk )
				{
						CAdmitTrackerAction * admitTrackerAction = new CAdmitTrackerAction( message.m_header.m_actionKey, key );
						admitTrackerAction->process_event( common::CMessageResult( message, key ) );
						common::CActionHandler::getInstance()->executeAction( admitTrackerAction );
				}
				else if ( message.m_header.m_payloadKind == common::CPayloadKind::EnterNetworkAsk )
				{
					CEnterNetworkAction * enterNetworkAction = new CEnterNetworkAction( message.m_header.m_actionKey, key );
					enterNetworkAction->process_event( common::CMessageResult( message, key ) );
					common::CActionHandler::getInstance()->executeAction( enterNetworkAction );
				}
				else if ( message.m_header.m_payloadKind == common::CPayloadKind::AckTransactions )
				{
					CAdmitTransactionBundle * admitTransactionBundle = new CAdmitTransactionBundle( message.m_header.m_actionKey );
					admitTransactionBundle->process_event( common::CMessageResult( message, key ) );
					common::CActionHandler::getInstance()->executeAction( admitTransactionBundle );
				}
				else if ( message.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
				{
					CProvideInfoAction * provideInfoAction= new CProvideInfoAction(
								 message.m_header.m_actionKey
								, key
								);

					provideInfoAction->process_event( common::CMessageResult( message, key ) );

					common::CActionHandler::getInstance()->executeAction( provideInfoAction );
				}
				else if ( message.m_header.m_payloadKind == common::CPayloadKind::SynchronizationAsk )
				{
					CSynchronizationAction * synchronizationAction= new CSynchronizationAction(
								  message.m_header.m_id
								, message.m_header.m_actionKey
								, key
								);

					common::CActionHandler::getInstance()->executeAction( synchronizationAction );
				}
				else if ( message.m_header.m_payloadKind == common::CPayloadKind::Transactions )
				{
					CAdmitTransactionBundle * admitTransactionBundle = new CAdmitTransactionBundle( message.m_header.m_actionKey );
					common::CActionHandler::getInstance()->executeAction( admitTransactionBundle );
					admitTransactionBundle->process_event( common::CMessageResult( message, key ) );
				}
				else if ( message.m_header.m_payloadKind == common::CPayloadKind::FullRankingInfo )
				{
					CUpdateNetworkDataAction * updateNetworkDataAction = new CUpdateNetworkDataAction( message.m_header.m_actionKey );
					common::CActionHandler::getInstance()->executeAction( updateNetworkDataAction );
					updateNetworkDataAction->process_event( common::CMessageResult( message, key ) );
				}
				else if ( message.m_header.m_payloadKind == common::CPayloadKind::ActivationStatus )
				{
					CActivityControllerAction * activityControllerAction = new CActivityControllerAction( message.m_header.m_actionKey );
					activityControllerAction->process_event( common::CMessageResult( message, key ) );
					common::CActionHandler::getInstance()->executeAction( activityControllerAction );
				}
				else if ( message.m_header.m_payloadKind == common::CPayloadKind::Ping )
				{
					CPingAction * pingAction = new CPingAction( message.m_header.m_actionKey );

					pingAction->process_event( common::CMessageResult( message, key ) );

					common::CActionHandler::getInstance()->executeAction( pingAction );
				}
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
