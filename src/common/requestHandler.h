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

namespace common
{
/*
memory leak but  right  now  I can  live  with  that

*/

template < class _Type >
class CRequestHandler
{
public:
	typedef DimsResponse DimsResponse;
	typedef  CMedium CMedium;
public:
	CRequestHandler( CMedium * _medium );

	bool getLastResponse( CRequest * _request, CRequestHandler ::DimsResponse & _response ) const;

	bool isProcessed( CRequest * _request ) const;

	void runRequests();

	void processMediumResponses();

	void deleteRequest( CRequest * );

	void clearLastResponse( CRequest * _request );

	bool operator==( CMedium const * _medium ) const;

	bool operator<( CMedium const * _medium ) const;

	void setRequest( CRequest * _request );

	bool operator<( CRequestHandler  const & _handler ) const;

	std::list< DimsResponse > getDirectActionResponse( CAction const * _action );// this is bad use but I don't know  what will really happen yet

	void deleteAction( CAction const * _action );
private:
	void setInvalid(){ m_valid = false; }
private:
	bool m_valid;

	std::vector<CRequest *> m_newRequest;

	std::multimap<CRequest const*,DimsResponse > m_processedRequests;

	CMedium * m_usedMedium;
};

template < class _Type >
CRequestHandler ::CRequestHandler( CMedium * _medium )
	: m_valid( true )
	, m_usedMedium( _medium )
{
	_medium->registerDeleteHook( boost::bind( &CRequestHandler ::setInvalid, this ) );
}

template < class _Type >
bool
CRequestHandler ::getLastResponse( CRequest * _request, CRequestHandler ::DimsResponse & _response ) const
{
	typename std::multimap<CRequest const*,DimsResponse >::const_iterator iterator;

	iterator = m_processedRequests.lower_bound( _request );
	if ( iterator == m_processedRequests.upper_bound( _request ) )
		return false;

	_response = iterator->second;
	return true;
}

template < class _Type >
void
CRequestHandler ::clearLastResponse( CRequest * _request )
{
	typename std::multimap<CRequest const*,DimsResponse >::iterator iterator;

	iterator = m_processedRequests.lower_bound( _request );
	if ( iterator == m_processedRequests.upper_bound( _request ) )
		return;

	m_processedRequests.erase( iterator );
}

template < class _Type >
void
CRequestHandler ::deleteRequest( CRequest * _request )
{
	m_processedRequests.erase( _request );

	if ( m_valid )
		m_usedMedium->deleteRequest( _request );
}

template < class _Type >
bool
CRequestHandler ::operator==( CMedium const * _medium ) const
{
	return m_usedMedium == _medium;
}

template < class _Type >
bool
CRequestHandler ::operator<( CMedium const * _medium ) const
{
	return (long long)m_usedMedium < (long long)_medium;
}

template < class _Type >
bool
CRequestHandler ::operator<( CRequestHandler  const & _handler ) const
{
	return this->m_usedMedium < _handler.m_usedMedium;
}

template < class _Type >
bool
CRequestHandler ::isProcessed( CRequest * _request ) const
{
	if ( m_processedRequests.find( _request ) != m_processedRequests.end() )
		return true;

	return false;
}

template < class _Type >
void
 CRequestHandler ::setRequest( CRequest * _request )
{
	m_newRequest.push_back( _request );
}

template < class _Type >
void
 CRequestHandler ::runRequests()
{
	if ( !m_valid )
		return;

	m_usedMedium->prepareMedium();
	BOOST_FOREACH( CRequest * request, m_newRequest )
	{
		request->accept( m_usedMedium );
	}
	m_newRequest.clear();
	m_usedMedium->flush();
}

template < class _Type >
std::list< DimsResponse >
CRequestHandler ::getDirectActionResponse( CAction const * _action )
{
	std::list< DimsResponse > responses;
	m_usedMedium->getDirectActionResponseAndClear( _action, responses );

	return responses;
}

template < class _Type >
void
CRequestHandler ::deleteAction( CAction const * _action )
{
	m_usedMedium->deleteAction( _action );
}

template < class _Type >
void
CRequestHandler ::processMediumResponses()
{
	if ( !m_valid )
		return;

	try
	{
		if( !m_usedMedium->serviced() )
			return;

		std::multimap< CRequest const*, DimsResponse > requestResponses;

		m_usedMedium->getResponseAndClear( requestResponses );

		BOOST_FOREACH( PAIRTYPE( CRequest const*, DimsResponse ) const & response, requestResponses )
		{
				m_processedRequests.insert( std::make_pair( response.first, response.second ) );
		}

	}
	 catch (CMediumException & _mediumException)
	{
// do  something  here
		BOOST_FOREACH( CRequest const* request, m_newRequest )
		{
		//	m_processedRequests.insert( std::make_pair( request, _mediumException ) );
		}
		// problem with synchronization here??
	}

}

}

#endif
