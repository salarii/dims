// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SELF_NODES_MANAGER_H
#define SELF_NODES_MANAGER_H

namespace Self
{

class CSelfNodesManager
{
public:
	CSelfNodesManager();

	void PropagateBundle( std::vector< CTransactions > _bundle );

	void handleMessages();
	
	void manageNodes();

	void connectNodes();
private:

	void propagateMessage();
	
	void analyseMessage();
};

}

#endif