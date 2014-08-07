// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef CONNECTION_PROVIDER_H
#define CONNECTION_PROVIDER_H

#include <list>

#include "request.h"

namespace common
{
template < class _RequestResponses >
class CMedium;

template < class _RequestResponses >
struct CAcceptFilter
{
	virtual std::list< CMedium< _RequestResponses > *> accepted() = 0;
};

template < class _RequestResponses >
struct CExcludeFilter
{
	virtual std::list< CMedium< _RequestResponses > *> notExcluded() = 0;
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
class CConnectionProvider
{
public:
	virtual std::list< CMedium< _RequestResponses > *> provideConnection( CMediumFilter< _RequestResponses > const & ) = 0;
};

}


#endif // CONNECTION_PROVIDER_H
