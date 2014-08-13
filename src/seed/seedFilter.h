#ifndef SEED_FILTER_H
#define SEED_FILTER_H

#include "common/filters.h"
#include "configureSeedActionHandler.h"

namespace seed
{

struct CSpecificMediumFilter : public common::CMediumFilter< SeedResponses >
{
	CSpecificMediumFilter( long long unsigned _ptr )
	: m_ptr( _ptr )
	{}

	std::list< common::CMedium< SeedResponses > *> getMediums( CSeedNodesManager * _nodesManager )const
	{
		std::list< common::CMedium< SeedResponses > *> mediums;

		mediums.push_back( _nodesManager->findNodeMedium( m_ptr ) );

		return mediums;
	}
	long long unsigned m_ptr;
};


struct CInternalMediumFilter : public common::CMediumFilter< SeedResponses >
{
	CInternalMediumFilter()
	{}

	std::list< common::CMedium< SeedResponses > *> getMediums( CSeedNodesManager * _nodesManager )const
	{
		return _nodesManager->getInternalMedium();
	}

};

}

#endif // SEED_FILTER_H
