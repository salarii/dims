// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "common/connectionProvider.h"
#include "common/communicationBuffer.h"
#include "common/nodeMessages.h"
#include "common/medium.h"
#include "common/support.h"
#include "common/requestHandler.h"

namespace common
{

CRequestHandler::CRequestHandler( CMedium * _medium )
	: m_valid( true )
	, m_usedMedium( _medium )
{
	_medium->registerDeleteHook( boost::bind( &CRequestHandler::setInvalid, this ) );
}

void
CRequestHandler::setInvalid()
{
	m_valid = false;
}

bool
CRequestHandler ::getLastResponse( CRequest * _request, common::DimsResponse & _response ) const
{
	boost::lock_guard<boost::mutex> lock(m_mutex);
	typename std::multimap<CRequest const*,DimsResponse >::const_iterator iterator;

	iterator = m_processedRequests.lower_bound( _request );
	if ( iterator == m_processedRequests.upper_bound( _request ) )
		return false;

	_response = iterator->second;
	return true;
}

void
CRequestHandler ::clearLastResponse( CRequest * _request )
{
	boost::lock_guard<boost::mutex> lock(m_mutex);
	typename std::multimap<CRequest const*,DimsResponse >::iterator iterator;

	iterator = m_processedRequests.lower_bound( _request );
	if ( iterator == m_processedRequests.upper_bound( _request ) )
		return;

	m_processedRequests.erase( iterator );
}

void
CRequestHandler ::deleteRequest( CRequest * _request )
{
	boost::lock_guard<boost::mutex> lock(m_mutex);

	m_processedRequests.erase( _request );

	if ( m_valid )
		m_usedMedium->deleteRequest( _request );
}

bool
CRequestHandler ::operator==( CMedium const * _medium ) const
{
	return m_usedMedium == _medium;
}

bool
CRequestHandler ::operator<( CMedium const * _medium ) const
{
	return (long long)m_usedMedium < (long long)_medium;
}

bool
CRequestHandler ::operator<( CRequestHandler  const & _handler ) const
{
	return this->m_usedMedium < _handler.m_usedMedium;
}

bool
CRequestHandler ::isProcessed( CRequest * _request ) const
{
	boost::lock_guard<boost::mutex> lock(m_mutex);

	if ( !m_valid )
		return false;

	if ( m_processedRequests.find( _request ) != m_processedRequests.end() )
		return true;

	return false;
}

void
 CRequestHandler ::setRequest( CRequest * _request )
{
	boost::lock_guard<boost::mutex> lock(m_mutex);

	m_newRequest.push_back( _request );
}

void
 CRequestHandler ::runRequests()
{
	boost::lock_guard<boost::mutex> lock(m_mutex);

	if ( !m_valid )
	{
		BOOST_FOREACH( CRequest * request, m_newRequest )
		{
			request->setProcessed();

			m_newRequest.clear();
		}
		return;
	}

	m_usedMedium->prepareMedium();
	BOOST_FOREACH( CRequest * request, m_newRequest )
	{
		request->setProcessed();
		request->accept( m_usedMedium );
	}
	m_newRequest.clear();
	m_usedMedium->flush();
}

std::list< DimsResponse >
CRequestHandler ::getDirectActionResponse( CAction const * _action )
{
	boost::lock_guard<boost::mutex> lock(m_mutex);
	std::list< DimsResponse > responses;

	if ( !m_valid )
		return responses;

	m_usedMedium->getDirectActionResponseAndClear( _action, responses );

	return responses;
}

void
CRequestHandler ::deleteAction( CAction const * _action )
{
	boost::lock_guard<boost::mutex> lock(m_mutex);
	if ( !m_valid )
		return;
	m_usedMedium->deleteAction( _action );
}

void
CRequestHandler ::processMediumResponses()
{
	boost::lock_guard<boost::mutex> lock(m_mutex);
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
