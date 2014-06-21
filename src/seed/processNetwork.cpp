// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "processNetwork.h"
#include "seedNodeMedium.h"
#include "seedNodesManager.h"
#include "acceptNodeAction.h"
#include "common/actionHandler.h"
#include "common/communicationProtocol.h"


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
		CSeedNodesManager::getInstance()->addNode( pfrom );
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

			CSeedNodeMedium * nodeMedium = CSeedNodesManager::getInstance()->getMediumForNode( pfrom );

			uint256 hash = Hash( &identifyMessage.m_payload.front(), &identifyMessage.m_payload.back() );

			if ( nodeMedium->isIdentifyMessageKnown( hash ) )
			{
				nodeMedium->setResponse( hash, common::CIdentificationResult( identifyMessage.m_payload, identifyMessage.m_signed, identifyMessage.m_key ) );
			}
			else
			{
				CAcceptNodeAction * connectTrackerAction= new CAcceptNodeAction( identifyMessage.m_payload, convertToInt( nodeMedium->getNode() ) );
				common::CActionHandler< SeedResponses >::getInstance()->executeAction( connectTrackerAction );

			}

		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::Uninitiated )
		{
			//
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