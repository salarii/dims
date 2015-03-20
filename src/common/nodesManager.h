// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef NODES_MANAGER_H
#define NODES_MANAGER_H

/*
very probable that this  should be in common
*/
#include "connectionProvider.h"
#include "selfNode.h"
#include "nodeMedium.h"

#include "manageNetwork.h"

#include "mediumKinds.h"

namespace common
{
// class common::CConnectionProvider is problematic, may providers make  things confusing, sometimes it is not  clear which provider is responsible for  mediums of specific
template < class _MediumFilter >
class CNodesManager : public common::CConnectionProvider< _MediumFilter >
{
public:
	typedef MEDIUM_TYPE(_MediumFilter) Medium;
public:
	bool getMessagesForNode( common::CSelfNode * _node, std::vector< common::CMessage > & _messages );

	bool processMessagesFromNode( common::CSelfNode * _node, std::vector< common::CMessage > const & _messages );

	void addNode( CNodeMedium< Medium > * _medium );

	std::list< Medium *> provideConnection( _MediumFilter const & _mediumFilter );

	virtual CNodeMedium< Medium >* getMediumForNode( common::CSelfNode * _node ) const;

	Medium * findNodeMedium( uintptr_t _ptr ) const;

	Medium * eraseMedium( uintptr_t _nodePtr );

	virtual std::list< Medium *> getNodesByClass( CMediumKinds::Enum _nodesClass ) const = 0;
protected:
	CNodesManager();

	mutable boost::mutex m_nodesLock;

	std::map< uintptr_t, CNodeMedium< Medium >* > m_ptrToNodes;
protected:
	static CNodesManager< _MediumFilter > * ms_instance;
};

template < class _MediumFilter >
CNodesManager< _MediumFilter >::CNodesManager()
{
}

template < class _MediumFilter >
void
CNodesManager< _MediumFilter >::addNode( CNodeMedium< Medium > * _medium )
{
	boost::lock_guard<boost::mutex> lock( m_nodesLock );
// create  and  run ping  action
	m_ptrToNodes.insert( std::make_pair( convertToInt( _medium->getNode() ), _medium ) );
}

template < class _MediumFilter >
CNodeMedium< MEDIUM_TYPE(_MediumFilter) >*
CNodesManager< _MediumFilter >::getMediumForNode( common::CSelfNode * _node ) const
{
	typename std::map< uintptr_t, CNodeMedium< Medium >* >::const_iterator iterator = m_ptrToNodes.find( convertToInt( _node ) );
	if ( iterator != m_ptrToNodes.end() )
	{
		return iterator->second;
	}

	return 0;
}

template < class _MediumFilter >
std::list< MEDIUM_TYPE(_MediumFilter) *>
CNodesManager< _MediumFilter >::provideConnection( _MediumFilter const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

template < class _MediumFilter >
MEDIUM_TYPE(_MediumFilter) *
CNodesManager< _MediumFilter >::findNodeMedium( uintptr_t _ptr ) const
{
	typename std::map< uintptr_t, CNodeMedium< Medium >* >::const_iterator iterator = m_ptrToNodes.find( _ptr );

	return iterator != m_ptrToNodes.end() ? iterator->second : 0;
}

template < class _MediumFilter >
MEDIUM_TYPE(_MediumFilter) *
CNodesManager< _MediumFilter >::eraseMedium( uintptr_t _nodePtr )
{
	typename std::map< uintptr_t, CNodeMedium< Medium >* >::iterator iterator = m_ptrToNodes.find( _nodePtr );

	if ( iterator == m_ptrToNodes.end() )
		return 0;

	Medium * medium = iterator->second;

	m_ptrToNodes.erase( iterator );

	return medium;
}

}

#endif // NODES_MANAGER_H
