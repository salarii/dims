// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include "tracker/validationManager.h"

#include "connectionProvider.h"

#include "requestResponse.h"

#include "common/nodeMessages.h"
#include "medium.h"
#include "support.h"

#include "common/communicationBuffer.h"

namespace common
{
template < class _RequestResponses >
class CMedium;

/*
memory leak but  right  now  I can  live  with  that
*/

template < class _RequestResponses >
class CRequestHandler
{
public:
	CRequestHandler( CMedium< _RequestResponses > * _medium );

	std::vector< _RequestResponses > getResponses( CRequest< _RequestResponses >* _request ) const;

	bool isProcessed( CRequest< _RequestResponses >* _request ) const;

	void setRequest( CRequest< _RequestResponses >* _request );

	void runRequests();

	void processMediumResponses();

	void deleteRequest( CRequest< _RequestResponses >* );

	bool operator==( CMedium< _RequestResponses > const * _medium ) const;

	bool operator<( CMedium< _RequestResponses > const * _medium ) const;

	bool operator<( CRequestHandler< _RequestResponses > const & _handler ) const;
private:
	std::vector<CRequest< _RequestResponses >*> m_newRequest;
	std::map<CRequest< _RequestResponses >*,uint256> m_pendingRequest;
	std::map<CRequest< _RequestResponses >*,std::vector< _RequestResponses > > m_processedRequests;

	CMedium< _RequestResponses > * m_usedMedium;
};

template < class _RequestResponses >
CRequestHandler< _RequestResponses >::CRequestHandler( CMedium< _RequestResponses > * _medium )
	:m_usedMedium( _medium )
{
}

template < class _RequestResponses >
std::vector< _RequestResponses >
CRequestHandler< _RequestResponses >::getResponses( CRequest< _RequestResponses >* _request ) const
{
	if( m_processedRequests.find( _request ) != m_processedRequests.end() )
		return m_processedRequests.find( _request )->second;
}

template < class _RequestResponses >
void
 CRequestHandler< _RequestResponses >::deleteRequest( CRequest< _RequestResponses >* _request )
{
	m_processedRequests.erase( _request );
}

template < class _RequestResponses >
bool
CRequestHandler< _RequestResponses >::operator==( CMedium< _RequestResponses > const * _medium ) const
{
	return m_usedMedium == _medium;
}

template < class _RequestResponses >
bool
CRequestHandler< _RequestResponses >::operator<( CMedium< _RequestResponses > const * _medium ) const
{
	return (long long)m_usedMedium < (long long)_medium;
}

template < class _RequestResponses >
bool
CRequestHandler< _RequestResponses >::operator<( CRequestHandler< _RequestResponses > const & _handler ) const
{
	return this->m_usedMedium < _handler.m_usedMedium;
}

template < class _RequestResponses >
bool
CRequestHandler< _RequestResponses >::isProcessed( CRequest< _RequestResponses >* _request ) const
{
	if ( m_processedRequests.find( _request ) != m_processedRequests.end() )
		return true;

	return false;
}

template < class _RequestResponses >
void
 CRequestHandler< _RequestResponses >::setRequest( CRequest< _RequestResponses >* _request )
{
	m_newRequest.push_back( _request );
}

template < class _RequestResponses >
void
 CRequestHandler< _RequestResponses >::runRequests()
{
	BOOST_FOREACH( CRequest< _RequestResponses >* request, m_newRequest )
	{
		request->accept( m_usedMedium );
	}

	m_usedMedium->flush();
}

template < class _RequestResponses >
void
 CRequestHandler< _RequestResponses >::processMediumResponses()
{
	try
	{
		if( !m_usedMedium->serviced() )
			return;

		std::map< CRequest< _RequestResponses >*, std::vector< _RequestResponses > > requestResponses;

		m_usedMedium->getResponseAndClear( requestResponses );

		assert( m_newRequest.size() == requestResponses.size() );// this  assert in general  is wrong but  it may be  useful for time being

		BOOST_FOREACH( PAIRTYPE( CRequest< _RequestResponses >*, std::vector< _RequestResponses > ) const & response, requestResponses )
		{
				m_processedRequests.insert( std::make_pair( response.first, response.second ) );
		}
		m_newRequest.clear();
	}
	 catch (CMediumException & _mediumException)
	{
//CSystemError is now  common for  every  action handler, it is neither  flexible  nor  good
// it heve to be redesigned  somehow
//right now, keep in mind that every single  action is responsible  for handling errors

		BOOST_FOREACH( CRequest< _RequestResponses >* request, m_newRequest )
		{
		//	m_processedRequests.insert( std::make_pair( request, _mediumException ) );
		}
		// problem with synchronization here??
		m_newRequest.clear();
	}

}

}

#endif
