// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "tracker/validationManager.h"

#include "connectionProvider.h"

#include "commonResponses.h"

#include "common/nodeMessages.h"
#include "medium.h"
#include "support.h"

#include "common/communicationBuffer.h"

namespace common
{
/*
memory leak but  right  now  I can  live  with  that

*/

template < class _Medium >
class CRequestHandler
{
public:
	typedef RESPONSE_TYPE(_Medium) Response;
public:
	CRequestHandler( _Medium * _medium );

	std::vector< Response > getResponses( CRequest< Response >* _request ) const;

	bool isProcessed( CRequest< Response >* _request ) const;

	void runRequests();

	void processMediumResponses();

	void deleteRequest( CRequest< Response >* );

	bool operator==( _Medium const * _medium ) const;

	bool operator<( _Medium const * _medium ) const;

	void setRequest( CRequest< Response >* _request );

	bool operator<( CRequestHandler< _Medium > const & _handler ) const;
private:
	std::vector<CRequest< Response >*> m_newRequest;
	std::map<CRequest< Response >*,uint256> m_pendingRequest;
	std::multimap<CRequest< Response >const*,Response > m_processedRequests;

	_Medium * m_usedMedium;
};

template < class _Medium >
CRequestHandler< _Medium >::CRequestHandler( _Medium * _medium )
	:m_usedMedium( _medium )
{
}

template < class _Medium >
std::vector< typename CRequestHandler< _Medium >::Response >
CRequestHandler< _Medium >::getResponses( CRequest< Response >* _request ) const
{
	typename std::multimap<CRequest< Response >const*,Response >::const_iterator iterator;

	std::vector< Response > responses;

	for ( iterator = m_processedRequests.lower_bound( _request ); iterator != m_processedRequests.upper_bound( _request ); ++iterator )
	{
		responses.push_back( iterator->second );
	}

	return responses;
}

template < class _Medium >
void
 CRequestHandler< _Medium >::deleteRequest( CRequest< Response >* _request )
{
	m_processedRequests.erase( _request );
}

template < class _Medium >
bool
CRequestHandler< _Medium >::operator==( _Medium const * _medium ) const
{
	return m_usedMedium == _medium;
}

template < class _Medium >
bool
CRequestHandler< _Medium >::operator<( _Medium const * _medium ) const
{
	return (long long)m_usedMedium < (long long)_medium;
}

template < class _Medium >
bool
CRequestHandler< _Medium >::operator<( CRequestHandler< _Medium > const & _handler ) const
{
	return this->m_usedMedium < _handler.m_usedMedium;
}

template < class _Medium >
bool
CRequestHandler< _Medium >::isProcessed( CRequest< Response >* _request ) const
{
	if ( m_processedRequests.find( _request ) != m_processedRequests.end() )
		return true;

	return false;
}

template < class _Medium >
void
 CRequestHandler< _Medium >::setRequest( CRequest< Response >* _request )
{
	m_newRequest.push_back( _request );
}

template < class _Medium >
void
 CRequestHandler< _Medium >::runRequests()
{
	m_usedMedium->prepareMedium();
	BOOST_FOREACH( CRequest< Response >* request, m_newRequest )
	{
		request->accept( m_usedMedium );
	}
	m_newRequest.clear();
	m_usedMedium->flush();
}

template < class _Medium >
void
 CRequestHandler< _Medium >::processMediumResponses()
{
	try
	{
		if( !m_usedMedium->serviced() )
			return;

		std::multimap< CRequest< Response >const*, Response > requestResponses;

		m_usedMedium->getResponseAndClear( requestResponses );

		BOOST_FOREACH( PAIRTYPE( CRequest< Response >const*, Response ) const & response, requestResponses )
		{
				m_processedRequests.insert( std::make_pair( response.first, response.second ) );
		}

	}
	 catch (CMediumException & _mediumException)
	{
// do  something  here
		BOOST_FOREACH( CRequest< Response >const* request, m_newRequest )
		{
		//	m_processedRequests.insert( std::make_pair( request, _mediumException ) );
		}
		// problem with synchronization here??
	}

}

}

#endif
