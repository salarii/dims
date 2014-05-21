// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODES_MANAGER_H
#define NODES_MANAGER_H

#include "communicationProtocol.h"
#include "common/connectionProvider.h"
#include "configureTrackerActionHandler.h"

class CNode;

namespace tracker
{
/*
 * crappy design
*/
class CNodeMedium;

template < class T >
unsigned int convertToInt( T * _t )
{
	return static_cast< unsigned int >( _t );
}

class CNodesManager : public common::CConnectionProvider< TrackerResponses >
{
public:
	bool getMessagesForNode( CSelfNode * _node, std::vector< CMessage > & _messages );

	bool processMessagesFormNode( CSelfNode * _node, std::vector< CMessage > const & _messages );

	void connectNodes();

	void addNode( CSelfNode * _node );

	bool isNodeHonest();

	bool isBanned( CAddress const & _address ); // address may be banned  when , associated  node  make   trouble

	std::list< common::CMedium< TrackerResponses > *> provideConnection( int const _actionKind, unsigned _requestedConnectionNumber = -1 );

	static CNodesManager * getInstance();
private:
	CNodesManager();

	void handleMessages();

	void propagateMessage();
	
	void analyseMessage();

	mutable boost::mutex m_nodesLock;

	//std::list< CSelfNode * > m_unidentified;

	std::map< unsigned int, CNodeMedium* > m_ptrToNodes;
private:
	static CNodesManager * ms_instance;

	std::list< common::CMedium< TrackerResponses > *> m_nodeMediums;
};

}

#endif
