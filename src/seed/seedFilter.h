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

	std::list< common::CMedium< SeedResponses > *> getMediums( common::CNodesManager< seed::SeedResponses > * _nodesManager )const
	{
		std::list< common::CMedium< SeedResponses > *> mediums;

		mediums.push_back( _nodesManager->findNodeMedium( m_ptr ) );

		return mediums;
	}
	long long unsigned m_ptr;
};


}

#endif // SEED_FILTER_H
