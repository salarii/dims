// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SELF_NODES_MANAGER_H
#define SELF_NODES_MANAGER_H

namespace tracker
{
/*
this  should  store  addresses  and  manage  conections  to other  nodes,  such  like  other  trackers  and  monitors 
this should  be also  store  on  relevant  information  about  known  nodes  such  like  fees

this in case when  nodes  can't  access  monitor because  of  huge  congestion
the effort  should be  made  to  decrease monitors overhead  since  thay  may be  performing  quite heavy  auditing  operations

*/
class CSelfNodesManager
{
public:
	CSelfNodesManager();

	void PropagateBundle( std::vector< CTransactions > _bundle );

	void handleMessages();
	
	void manageNodes();

	void connectNodes();

	bool isNodeHonest();

	bool isBanned( CAddress const & _address ); // address may be banned  when , associated  node  make   trouble
private:

	void propagateMessage();
	
	void analyseMessage();
};

}

#endif