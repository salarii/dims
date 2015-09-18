// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODES_MANAGER_H
#define NODES_MANAGER_H

#include "common/connectionProvider.h"
#include "common/selfNode.h"
#include "common/nodeMedium.h"
#include "common/manageNetwork.h"
#include "common/mediumKinds.h"

namespace common
{
// class common::CConnectionProvider is problematic, may providers make  things confusing, sometimes it is not  clear which provider is responsible for  mediums of specific

class CNodesManager : public common::CConnectionProvider
{
public:
	bool getMessagesForNode( common::CSelfNode * _node, std::vector< common::CMessage > & _messages );

	bool processMessagesFromNode( common::CSelfNode * _node, std::vector< common::CMessage > const & _messages );

	void addNode( CNodeMedium * _medium );

	std::list< CMedium *> provideConnection( CMediumFilter const & _mediumFilter );

	virtual CNodeMedium* getMediumForNode( common::CSelfNode * _node ) const;

	CMedium * findNodeMedium( uintptr_t _ptr ) const;

		virtual void eraseMedium( uintptr_t _nodePtr );

	virtual std::list< CMedium *> getNodesByClass( CMediumKinds::Enum _nodesClass ) const = 0;

	bool getAddress( uintptr_t _nodePtr, CAddress & _address ) const;

	static CNodesManager * getInstance()
	{
		return ms_instance;
	}
protected:
	CNodesManager();

	mutable boost::mutex m_nodesLock;

	std::map< uintptr_t, CNodeMedium* > m_ptrToNodes;
protected:
	static CNodesManager * ms_instance;
};

CNodesManager ::CNodesManager()
{
}

void
CNodesManager ::addNode( CNodeMedium * _medium )
{
	boost::lock_guard<boost::mutex> lock( m_nodesLock );
// create  and  run ping  action
	m_ptrToNodes.insert( std::make_pair( convertToInt( _medium->getNode() ), _medium ) );
}

CNodeMedium*
CNodesManager ::getMediumForNode( common::CSelfNode * _node ) const
{
	typename std::map< uintptr_t, CNodeMedium* >::const_iterator iterator = m_ptrToNodes.find( convertToInt( _node ) );
	if ( iterator != m_ptrToNodes.end() )
	{
		return iterator->second;
	}

	return 0;
}

std::list< CMedium*>
CNodesManager ::provideConnection( CMediumFilter const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

 CMedium *
CNodesManager ::findNodeMedium( uintptr_t _ptr ) const
{
	typename std::map< uintptr_t, CNodeMedium* >::const_iterator iterator = m_ptrToNodes.find( _ptr );

	return iterator != m_ptrToNodes.end() ? iterator->second : 0;
}

void
CNodesManager ::eraseMedium( uintptr_t _nodePtr )
{
	m_ptrToNodes.erase( _nodePtr );
}

bool
CNodesManager ::getAddress( uintptr_t _nodePtr, CAddress & _address ) const
{
	typename std::map< uintptr_t, CNodeMedium* >::const_iterator iterator = m_ptrToNodes.find( _nodePtr );

	if ( iterator == m_ptrToNodes.end() )
		return false;

	_address = iterator->second->getNode()->addr;

	return true;
}

}

#endif // NODES_MANAGER_H
