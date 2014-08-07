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
struct CExcludeFilter
{
	virtual ~CExcludeFilter(){};

	virtual std::list< CMedium< _RequestResponses > *> notExcluded( std::list< CMedium< _RequestResponses > *> const & _candidates ) = 0;
};

template < class _RequestResponses >
struct CMediumFilter
{
	CMediumFilter( int _mediumClass = -1, int _mediumNumber = -1, CAcceptFilter< _RequestResponses > * _accept = 0, CExcludeFilter< _RequestResponses > * _exclude = 0 ):
		m_mediumClass( _mediumClass ),
		m_mediumNumber( _mediumNumber ),
		m_accept( _accept ),
		m_exclude( _exclude )
	{}

	~CMediumFilter()
	{
		if ( m_accept )
			delete m_accept;

		if( m_exclude )
			delete m_exclude;
	}

	int m_mediumClass;
	int m_mediumNumber;

	CAcceptFilter< _RequestResponses >* m_accept;
	CExcludeFilter< _RequestResponses >* m_exclude;
};

template < class _RequestResponses >
struct CAcceptFilterByShortPtr : public CAcceptFilter< _RequestResponses >
{
	CAcceptFilterByShortPtr( int _shortPtr )
		:m_shortPtr( _shortPtr )
	{
	}

	std::list< CMedium< _RequestResponses > *> accepted( std::list< CMedium< _RequestResponses > *> const & _candidates )
	{
		std::list< CMedium< _RequestResponses > *> acceptedMedium;

		BOOST_FOREACH( CMedium< _RequestResponses > * medium, _candidates )
		{
			if ( convertToInt( medium ) == m_shortPtr )
			{
				acceptedMedium.push_back( medium );
			}
		}
	}

	int m_shortPtr;
};

}

#endif // FILTERS_H
