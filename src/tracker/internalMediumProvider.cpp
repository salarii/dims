// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "internalMediumProvider.h"
#include "internalOperationsMedium.h"
#include "bitcoinNodeMedium.h"

namespace tracker
{

CInternalMediumProvider * CInternalMediumProvider::ms_instance = NULL;

CInternalMediumProvider*
CInternalMediumProvider::getInstance( )
{
	if ( !ms_instance )
	{
		ms_instance = new CInternalMediumProvider();
	};
	return ms_instance;
}

CInternalMediumProvider::CInternalMediumProvider()
{
	m_mediums.push_back( CInternalOperationsMedium::getInstance() );
}

std::list< common::CMedium< TrackerResponses > *>
CInternalMediumProvider::provideConnection( common::CMediumFilter< TrackerResponses > const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}


std::list< common::CMedium< TrackerResponses > *>
CInternalMediumProvider::getMediumByClass( common::CMediumKinds::Enum _mediumKind, unsigned int _mediumNumber )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	if ( common::CMediumKinds::Internal == _mediumKind )
		return m_mediums;
	else if ( common::CMediumKinds::BitcoinsNodes == _mediumKind )
	{
		std::list< common::CMedium< TrackerResponses > *> mediums;

		std::map< CNode *, CBitcoinNodeMedium * >::const_iterator iterator =  m_nodeToMedium.begin();
		//simplified  approach
		for ( unsigned int i = 0; ( i < vNodes.size() ) && ( i < _mediumNumber ); )
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
CInternalMediumProvider::setResponse( CTransaction const & _response, CNode * _node )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	std::map< CNode *, CBitcoinNodeMedium * >::iterator iterator = m_nodeToMedium.find( _node );

	if( iterator == m_nodeToMedium.end() ) return;// not  asked

	iterator->second->setResponse( _response );
}

void
CInternalMediumProvider::setResponse( CMerkleBlock const & _merkle, CNode * _node )
{
	boost::lock_guard<boost::mutex> lock( m_mutex );

	std::map< CNode *, CBitcoinNodeMedium * >::iterator iterator = m_nodeToMedium.find( _node );

	if( iterator == m_nodeToMedium.end() ) return;// not  asked

	iterator->second->setResponse( _merkle );
}

}
