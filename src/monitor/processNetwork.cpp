// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "processNetwork.h"
#include "common/nodesManager.h"
#include "common/communicationProtocol.h"
#include "common/actionHandler.h"

#include "common/nodeMedium.h"
#include "configureMonitorActionHandler.h"

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
	if ( !common::CNodesManager<MonitorResponses>::getInstance()->getMediumForNode( pfrom ) )
	{
		common::CNodesManager<MonitorResponses>::getInstance()->addNode( pfrom );
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

			common::CNodeMedium<MonitorResponses> * nodeMedium = common::CNodesManager<MonitorResponses>::getInstance()->getMediumForNode( pfrom );

			uint256 hash = Hash( &identifyMessage.m_payload.front(), &identifyMessage.m_payload.back() );

			if ( nodeMedium->isIdentifyMessageKnown( hash ) )
			{
			//	nodeMedium->setResponse( hash, common::CIdentificationResult<MonitorResponses>( identifyMessage.m_payload, identifyMessage.m_signed, identifyMessage.m_key ) );
			}
			else
			{
				/*CConnectTrackerAction * connectTrackerAction= new CConnectTrackerAction( identifyMessage.m_payload, convertToInt( nodeMedium->getNode() ) );
				common::CActionHandler< TrackerResponses >::getInstance()->executeAction( connectTrackerAction );*/

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

