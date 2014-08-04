// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "processNetwork.h"
#include "seedNodeMedium.h"
#include "seedNodesManager.h"
#include "acceptNodeAction.h"
#include "common/actionHandler.h"
#include "common/communicationProtocol.h"

#include <iostream>
#include <fstream>
using namespace std;


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

	ofstream myfile;
	 myfile.open ("test.txt", ios::app);



	std::vector< common::CMessage > messages;

		 myfile << "before\n";
myfile <<  vRecv.size()<<"\n";
		 myfile.close();
	vRecv >> messages;
myfile.open ("test.txt", ios::app);
		 myfile << "after\n";


// it is  stupid  to call this over and over again
	if ( !CSeedNodesManager::getInstance()->getMediumForNode( pfrom ) )
	{
		CSeedNodesManager::getInstance()->addNode( pfrom );
	}

	BOOST_FOREACH( common::CMessage const & message, messages )
	{
				 myfile << "type\n";
				 myfile << (int)message.m_header.m_payloadKind;
				 myfile.close();
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

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( identifyMessage.m_actionKey ) )
			{
				nodeMedium->setResponse( identifyMessage.m_actionKey, common::CIdentificationResult( identifyMessage.m_payload, identifyMessage.m_signed, identifyMessage.m_key, pfrom->addr ) );
			}
			else
			{
				CAcceptNodeAction * connectTrackerAction= new CAcceptNodeAction( identifyMessage.m_actionKey, identifyMessage.m_payload, convertToInt( nodeMedium->getNode() ) );
				common::CActionHandler< SeedResponses >::getInstance()->executeAction( connectTrackerAction );
			}

		}
		else if ( message.m_header.m_payloadKind == common::CPayloadKind::Uninitiated )
		{
			//
		}
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::RoleInfo )
		{
				 myfile.open ("test.txt", ios::app);
			CPubKey pubKey;
			if ( !CSeedNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) )
				;//service  error  somehow, can't  decode  action  at  this point  so it  have  to  be  done as  common  solution  for  all  such  issues
			common::CMessage orginalMessage;
			myfile << "befor  unwind\n";
			common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey );

			common::CNetworkRole networkRole;
myfile << "messae unwind\n";
			common::convertPayload( orginalMessage, networkRole );
myfile << "converted\n";
			CSeedNodeMedium * nodeMedium = CSeedNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( networkRole.m_actionKey ) )
			{
				myfile << "is serviced\n";
				nodeMedium->setResponse( networkRole.m_actionKey, common::CRoleResult( networkRole.m_role ) );
			}
			else
			{
								 myfile << "exception\n";
				assert(!"it should be existing action");

			}

							 myfile.close();
		}
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::NetworkInfo )
		{
			CPubKey pubKey;
			if ( !CSeedNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) )
				;

			common::CMessage orginalMessage;
			common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey );

			common::CKnownNetworkInfo knownNetworkInfo;

			common::convertPayload( orginalMessage, knownNetworkInfo );

			CSeedNodeMedium * nodeMedium = CSeedNodesManager::getInstance()->getMediumForNode( pfrom );

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
			if ( !CSeedNodesManager::getInstance()->getPublicKey( pfrom->addr, pubKey ) )
				;

			common::CMessage orginalMessage;
			common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey );

			common::CAck ack;

			common::convertPayload( orginalMessage, ack );

			CSeedNodeMedium * nodeMedium = CSeedNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( ack.m_actionKey ) )
			{
				nodeMedium->setResponse( ack.m_actionKey, common::CAckResult() );
			}
			else
			{
				assert(!"it should be existing action");

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
