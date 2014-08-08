#ifndef FILTERS_H
#define FILTERS_H

#include "connectionProvider.h"
#include "support.h"

#include "boost/foreach.hpp"

namespace common
{

template < class _RequestResponses >
struct CAcceptFilter
{
	virtual ~CAcceptFilter(){};

	virtual std::list< CMedium< _RequestResponses > *> accepted( std::list< CMedium< _RequestResponses > *> const & _candidates ) = 0;
};

template < class _RequestResponses >
struct CMediumFilter
{
	CMediumFilter( int _mediumClass = -1, int _mediumNumber = -1, CAcceptFilter< _RequestResponses > * _accept = 0 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber ),
		m_accept( _accept )
	{}

	~CMediumFilter()
	{
		if ( m_accept )
			delete m_accept;
	}

	int m_mediumClass;
	int m_mediumNumber;

	CAcceptFilter< _RequestResponses >* m_accept;
};

template < class _RequestResponses >
struct CAcceptFilterByPtr : public CAcceptFilter< _RequestResponses >
{
	CAcceptFilterByPtr( unsigned long long _ptr )
		:m_ptr( _ptr )
	{
	}

	std::list< CMedium< _RequestResponses > *> accepted( std::list< CMedium< _RequestResponses > *> const & _candidates )
	{
		std::list< CMedium< _RequestResponses > *> acceptedMedium;

		BOOST_FOREACH( CMedium< _RequestResponses > * medium, _candidates )
		{
			if ( convertToInt( medium ) == m_ptr )
			{
				acceptedMedium.push_back( medium );
			}
		}
	}

	unsigned long long m_ptr;
};

template < class _RequestResponses >
CMediumFilter< _RequestResponses > *
createFilterWithPtr( int _mediumClass, int _mediumNumber, unsigned long long _ptr )
{
	return new common::CMediumFilter< _RequestResponses >( _mediumClass, _mediumNumber, new common::CAcceptFilterByPtr< _RequestResponses >( _ptr ) );
}

}

#endif // FILTERS_H
