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
	bool getMessagesForNode( common::CSelfNode * _node, std::vector< common::CMessage > & _messages );

	bool processMessagesFromNode( common::CSelfNode * _node, std::vector< common::CMessage > const & _messages );

	void addNode( CNodeMedium< MEDIUM_TYPE(_MediumFilter) > * _medium );

	std::list< MEDIUM_TYPE(_MediumFilter) *> provideConnection( CMediumFilter< MEDIUM_TYPE(_MediumFilter) > const & _mediumFilter );

	virtual CNodeMedium< MEDIUM_TYPE(_MediumFilter) >* getMediumForNode( common::CSelfNode * _node ) const;

	MEDIUM_TYPE(_MediumFilter) * findNodeMedium( uintptr_t _ptr ) const;

	virtual std::list< MEDIUM_TYPE(_MediumFilter) *> getNodesByClass( CMediumKinds::Enum _nodesClass ) const = 0;
protected:
	CNodesManager();

	mutable boost::mutex m_nodesLock;

	std::map< unsigned int, CNodeMedium< MEDIUM_TYPE(_MediumFilter) >* > m_ptrToNodes;
protected:
	static CNodesManager< _MediumFilter > * ms_instance;
};

template < class _MediumFilter >
CNodesManager< _MediumFilter >::CNodesManager()
{
}

template < class _MediumFilter >
void
CNodesManager< _MediumFilter >::addNode( CNodeMedium< MEDIUM_TYPE(_MediumFilter) > * _medium )
{
	boost::lock_guard<boost::mutex> lock( m_nodesLock );

	m_ptrToNodes.insert( std::make_pair( convertToInt( _medium->getNode() ), _medium ) );
}

template < class _MediumFilter >
CNodeMedium< MEDIUM_TYPE(_MediumFilter) >*
CNodesManager< _MediumFilter >::getMediumForNode( common::CSelfNode * _node ) const
{
	typename std::map< unsigned int, CNodeMedium< MEDIUM_TYPE(_MediumFilter) >* >::const_iterator iterator = m_ptrToNodes.find( convertToInt( _node ) );
	if ( iterator != m_ptrToNodes.end() )
	{
		return iterator->second;
	}

	return 0;
}

template < class _MediumFilter >
std::list< MEDIUM_TYPE(_MediumFilter) *>
CNodesManager< _MediumFilter >::provideConnection( CMediumFilter< MEDIUM_TYPE(_MediumFilter) > const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}

template < class _MediumFilter >
MEDIUM_TYPE(_MediumFilter) *
CNodesManager< _MediumFilter >::findNodeMedium( uintptr_t _ptr ) const
{
	typename std::map< unsigned int, CNodeMedium< MEDIUM_TYPE(_MediumFilter) >* >::const_iterator iterator = m_ptrToNodes.find( _ptr );

	return iterator != m_ptrToNodes.end() ? iterator->second : 0;
}

}

#endif // NODES_MANAGER_H
