// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/actionHandler.h"
#include "common/communicationProtocol.h"
#include "common/events.h"
#include "common/networkActionRegister.h"

<<<<<<< HEAD
#include <iostream>
#include <fstream>
using namespace std;

=======
#include "seed/processNetwork.h"
#include "seed/seedNodeMedium.h"
#include "seed/seedNodesManager.h"
#include "seed/acceptNodeAction.h"
#include "seed/pingAction.h"
>>>>>>> 46651abfc2d0e9140b4ec774179ba6ab536f3a21

namespace seed
{
	CServiceResult service;//ugly
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
		CSeedNodesManager::getInstance()->addNode( new CSeedNodeMedium( pfrom ) );
	}

	BOOST_FOREACH( common::CMessage const & message, messages )
	{
<<<<<<< HEAD
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
=======
		if ( message.m_header.m_payloadKind == common::CPayloadKind::IntroductionReq )
>>>>>>> 46651abfc2d0e9140b4ec774179ba6ab536f3a21
		{
			common::CIdentifyMessage identifyMessage;
			convertPayload( message, identifyMessage );

			common::CNodeMedium * nodeMedium = CSeedNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->addActionResponse(
							message.m_header.m_actionKey
							, common::CIdentificationResult(
								  identifyMessage.m_payload
								, identifyMessage.m_signed
								, identifyMessage.m_key
								, pfrom->addr
								, message.m_header.m_id ) );
			}
			else
			{
<<<<<<< HEAD
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
			myfile << "time" << GetTime() << ":"  << message.m_header.m_time;
					 myfile.close();
			if ( !common::CommunicationProtocol::unwindMessage( message, orginalMessage, GetTime(), pubKey ) )
			{
				assert( !"this assert  should be  replaced by some logic for now to indicate problem" );
			}

			common::CNetworkRole networkRole;
myfile << "messae unwind\n";
myfile << "network role:"<< networkRole.GetSerializeSize( SER_DISK, CLIENT_VERSION )<< "\n";
myfile << "payload  size:" << orginalMessage.m_payload.size()<< "\n";
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
=======
				CAcceptNodeAction * connectNodeAction = new CAcceptNodeAction(
							message.m_header.m_actionKey, convertToInt( nodeMedium->getNode() ), service );

				connectNodeAction->process_event( common::CSwitchToConnectedEvent() );
				connectNodeAction->process_event( common::CIdentificationResult( identifyMessage.m_payload, identifyMessage.m_signed, identifyMessage.m_key, pfrom->addr, message.m_header.m_id ) );
				common::CActionHandler::getInstance()->executeAction( connectNodeAction );
			}

>>>>>>> 46651abfc2d0e9140b4ec774179ba6ab536f3a21
		}
		else if (  message.m_header.m_payloadKind == common::CPayloadKind::Ack )
		{
			common::CAck ack;

			common::convertPayload( message, ack );

			common::CNodeMedium * nodeMedium = CSeedNodesManager::getInstance()->getMediumForNode( pfrom );

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				nodeMedium->setResponse( message.m_header.m_id, common::CAckResult( convertToInt( nodeMedium->getNode() ) ) );
			}
		}
		else
		{

			common::CNodeMedium * nodeMedium = CSeedNodesManager::getInstance()->getMediumForNode( pfrom );

			CPubKey pubKey;

			if ( !CSeedNodesManager::getInstance()->getNodePublicKey( convertToInt( pfrom ), pubKey ) )
				return true;

			if ( common::CNetworkActionRegister::getInstance()->isServicedByAction( message.m_header.m_actionKey ) )
			{
				if ( message.m_header.m_payloadKind == common::CPayloadKind::InfoReq )
					nodeMedium->addActionResponse( message.m_header.m_actionKey, common::CMessageResult( message, pubKey ) );
				else
					nodeMedium->setResponse( message.m_header.m_id, common::CMessageResult( message, pubKey ) );
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
