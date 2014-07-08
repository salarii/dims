// Copyright (c) 2014 Dims dev-team
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

// for now  this will work in one thread in  blocking mode
// I want to avoid creating to much thread when  existing amount seems  to be sufficient
// this  class has to handle deny of  service somehow, most likely throwing  exception or  reloading  medium directly

template < class _RequestResponses >
class CRequestHandler
{
public:
	CRequestHandler( CMedium< _RequestResponses > * _medium );

	_RequestResponses getResponse( CRequest< _RequestResponses >* _request ) const;

	bool isProcessed( CRequest< _RequestResponses >* _request ) const;

	void setRequest( CRequest< _RequestResponses >* _request );

	void runRequests();

	void readLoop();

	void deleteRequest( CRequest< _RequestResponses >* );

	bool operator==( CMedium< _RequestResponses > const * _medium ) const;
private:
	std::vector<CRequest< _RequestResponses >*> m_newRequest;
	std::map<CRequest< _RequestResponses >*,uint256> m_pendingRequest;
	std::map<CRequest< _RequestResponses >*,_RequestResponses> m_processedRequests;

	CMedium< _RequestResponses > * m_usedMedium;
};

template < class _RequestResponses >
CRequestHandler< _RequestResponses >::CRequestHandler( CMedium< _RequestResponses > * _medium )
	:m_usedMedium( _medium )
{
}

template < class _RequestResponses >
_RequestResponses
 CRequestHandler< _RequestResponses >::getResponse( CRequest< _RequestResponses >* _request ) const
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
 CRequestHandler< _RequestResponses >::readLoop()
{
	try
	{
		while(!m_usedMedium->serviced());

		std::vector< _RequestResponses > requestResponses;
		/*
		there is  time gap  between getResponse and clearResponses

		it may cause deletion of valid response, if something  will arrive when getResponse releases a lock

		most probably I need to merge those  two  functions into one (non const)
		*/
		m_usedMedium->getResponse(requestResponses);
		m_usedMedium->clearResponses();
		// this i looks  ugly
		int i = 0;
		BOOST_FOREACH( _RequestResponses const & response, requestResponses )
		{
			m_processedRequests.insert( std::make_pair( m_newRequest[i++], response ) );
		}
		m_newRequest.clear();
	}
	 catch (CMediumException & _mediumException)
	{
//CSystemError is now  common for  every  action handler, it is neither  flexible  nor  good
// in order to change    this  I have  to redesign global error passing  functionality
//right now, keep in mind that every single  action is responsible  for handling errors

		BOOST_FOREACH( CRequest< _RequestResponses >* request, m_newRequest )
		{
			m_processedRequests.insert( std::make_pair( request, _mediumException ) );
		}
		m_newRequest.clear();
	}

}

}

#endif
