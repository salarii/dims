// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODES_MANAGER_H
#define NODES_MANAGER_H

#include "communicationProtocol.h"

class CNode;

namespace tracker
{
/*
this  should  store  addresses  and  manage  conections  to other  nodes,  such  like  other  trackers  and  monitors 
this should  be also  store  on  relevant  information  about  known  nodes  such  like  fees

this in case when  nodes  can't  access  monitor because  of  huge  congestion
the effort  should be  made  to  decrease monitors overhead  since  thay  may be  performing  quite heavy  auditing  operations

*/


class CNodesManager
{
public:
	void PropagateBundle( std::vector< CTransaction > _bundle );
	
	bool getMessagesForNode( CNode * _node, std::vector< CMessage > & _messages );

	bool processMessagesFormNode( CNode * _node, std::vector< CMessage > const & _messages );

	void connectNodes();

	bool isNodeHonest();

	bool isBanned( CAddress const & _address ); // address may be banned  when , associated  node  make   trouble

	static CNodesManager * getInstance( );
private:
	CNodesManager();

	void handleMessages();

	void propagateMessage();
	
	void analyseMessage();
private:
	static CNodesManager * ms_instance;
};

}

#endif
