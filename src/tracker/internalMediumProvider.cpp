// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "internalMediumProvider.h"
#include "internalOperationsMedium.h"
#include "trackerMediumsKinds.h"
#include "bitcoinNodeMedium.h"

namespace tracker
{

CInternalMediumProvider::CInternalMediumProvider()
{
	m_mediums.push_back( CInternalOperationsMedium::getInstance() );
}

std::list< common::CMedium< TrackerResponses > *>
CInternalMediumProvider::provideConnection( int const _actionKind, unsigned _requestedConnectionNumber )
{
	if ( CTrackerMediumsKinds::Internal == _actionKind )
		return m_mediums;
	else if ( CTrackerMediumsKinds::Nodes == _actionKind )
	{
		std::list< common::CMedium< TrackerResponses > *> mediums;

		std::map< CNode *, CBitcoinNodeMedium * >::iterator iterator =  m_nodeToMedium.begin();
		//simplified  approach
		for ( unsigned int i = 0; ( i < vNodes.size() ) && ( i < _requestedConnectionNumber ); )
		{

			if ( iterator != m_nodeToMedium.end() )
			{
				// validate that node  is  still working??
				mediums.push_back( static_cast< common::CMedium< TrackerResponses > * >( iterator->second ) );
				iterator++;
				++i;
			}
			else
			{
				CNode * node = vNodes.at( i );
				m_nodeToMedium.insert( std::make_pair( node, new CBitcoinNodeMedium( node ) ) );
				//ugly
				iterator =  m_nodeToMedium.begin();
				std::advance( iterator, i );
			}

		}

		return mediums;
	}
	else
		return std::list< common::CMedium< TrackerResponses > *>();
}

void
CInternalMediumProvider::setResponse( TrackerResponses const & _response, CNode * _node )
{
	std::map< CNode *, CBitcoinNodeMedium * >::iterator iterator = m_nodeToMedium.find( _node );

	iterator->second->setResponse( _response );
}

}
