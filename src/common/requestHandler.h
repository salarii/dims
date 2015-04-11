// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "tracker/validationManager.h"

#include "common/connectionProvider.h"
#include "commonResponses.h"
#include "common/communicationBuffer.h"
#include "common/nodeMessages.h"
#include "common/medium.h"
#include "common/support.h"
#include "common/types.h"

namespace common
{
/*
memory leak but  right  now  I can  live  with  that

*/

template < class _Type >
class CRequestHandler
{
public:
	typedef typename _Type::Response Response;
	typedef typename _Type::Medium Medium;
public:
	CRequestHandler( Medium * _medium );

	std::vector< Response > getResponses( CRequest< _Type >* _request ) const;

	bool isProcessed( CRequest< _Type >* _request ) const;

	void runRequests();

	void processMediumResponses();

	void deleteRequest( CRequest< _Type >* );

	bool operator==( Medium const * _medium ) const;

	bool operator<( Medium const * _medium ) const;

	void setRequest( CRequest< _Type >* _request );

	bool operator<( CRequestHandler< _Type > const & _handler ) const;

	std::list< typename _Type::Response > getDirectActionResponse( CAction< _Type >const * _action );// this is bad use but I don't know  what will really happen yet

	void deleteAction( CAction< _Type >const * _action );
private:
	void setInvalid(){ m_valid = false; }
private:
	bool m_valid;

	std::vector<CRequest< _Type >*> m_newRequest;

	std::multimap<CRequest< _Type >const*,Response > m_processedRequests;

	Medium * m_usedMedium;
};

template < class _Type >
CRequestHandler< _Type >::CRequestHandler( Medium * _medium )
	: m_usedMedium( _medium )
	, m_valid( true )
{
	_medium->registerDeleteHook( boost::bind( &CRequestHandler< _Type >::setInvalid, this ) );
}

template < class _Type >
std::vector< typename CRequestHandler< _Type >::Response >
CRequestHandler< _Type >::getResponses( CRequest< _Type >* _request ) const
{
	typename std::multimap<CRequest< _Type >const*,Response >::const_iterator iterator;

	std::vector< Response > responses;

	for ( iterator = m_processedRequests.lower_bound( _request ); iterator != m_processedRequests.upper_bound( _request ); ++iterator )
	{
		responses.push_back( iterator->second );
	}

	return responses;
}

template < class _Type >
void
 CRequestHandler< _Type >::deleteRequest( CRequest< _Type >* _request )
{
	m_processedRequests.erase( _request );

	if ( m_valid )
		m_usedMedium->deleteRequest( _request );
}

template < class _Type >
bool
CRequestHandler< _Type >::operator==( Medium const * _medium ) const
{
	return m_usedMedium == _medium;
}

template < class _Type >
bool
CRequestHandler< _Type >::operator<( Medium const * _medium ) const
{
	return (long long)m_usedMedium < (long long)_medium;
}

template < class _Type >
bool
CRequestHandler< _Type >::operator<( CRequestHandler< _Type > const & _handler ) const
{
	return this->m_usedMedium < _handler.m_usedMedium;
}

template < class _Type >
bool
CRequestHandler< _Type >::isProcessed( CRequest< _Type >* _request ) const
{
	if ( m_processedRequests.find( _request ) != m_processedRequests.end() )
		return true;

	return false;
}

template < class _Type >
void
 CRequestHandler< _Type >::setRequest( CRequest< _Type >* _request )
{
	m_newRequest.push_back( _request );
}

template < class _Type >
void
 CRequestHandler< _Type >::runRequests()
{
	if ( !m_valid )
		return;

	m_usedMedium->prepareMedium();
	BOOST_FOREACH( CRequest< _Type >* request, m_newRequest )
	{
		request->accept( m_usedMedium );
	}
	m_newRequest.clear();
	m_usedMedium->flush();
}

template < class _Type >
std::list< typename _Type::Response >
CRequestHandler< _Type >::getDirectActionResponse( CAction< _Type >const * _action )
{
	bool responseVaid;
	std::list< typename _Type::Response > responses;
	responseVaid = m_usedMedium->getDirectActionResponseAndClear( _action, responses );

	return responses;
}

template < class _Type >
void
CRequestHandler< _Type >::deleteAction( CAction< _Type >const * _action )
{
	m_usedMedium->deleteAction( _action );
}

template < class _Type >
void
CRequestHandler< _Type >::processMediumResponses()
{
	if ( !m_valid )
		return;

	try
	{
		if( !m_usedMedium->serviced() )
			return;

		std::multimap< CRequest< _Type >const*, Response > requestResponses;

		m_usedMedium->getResponseAndClear( requestResponses );

		BOOST_FOREACH( PAIRTYPE( CRequest< _Type >const*, Response ) const & response, requestResponses )
		{
				m_processedRequests.insert( std::make_pair( response.first, response.second ) );
		}

	}
	 catch (CMediumException & _mediumException)
	{
// do  something  here
		BOOST_FOREACH( CRequest< _Type >const* request, m_newRequest )
		{
		//	m_processedRequests.insert( std::make_pair( request, _mediumException ) );
		}
		// problem with synchronization here??
	}

}

}

#endif
