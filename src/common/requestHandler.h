// Copyright (c) 2014 Ratcoin dev-team
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

class CMedium;

// for now  this will work in one thread in  blocking mode
// I want to avoid creating to much thread when  existing amount seems  to be sufficient
// this  class has to handle deny of  service somehow, most likely throwing  exception or  reloading  medium directly

template < class _RequestResponses >
class CRequestHandler
{
public:
	CRequestHandler( CMedium * _medium );

	_RequestResponses getResponse( CRequest* _request ) const;

	bool isProcessed( CRequest* _request ) const;

	bool setRequest( CRequest* _request );

	void runRequests();

	void readLoop();

	void deleteRequest( CRequest* );
private:
	std::vector<CRequest*> m_newRequest;
	std::map<CRequest*,uint256> m_pendingRequest;
	std::map<CRequest*,_RequestResponses> m_processedRequests;

	CMedium * m_usedMedium;
};

template < class _RequestResponses >
CRequestHandler< _RequestResponses >::CRequestHandler( CMedium * _medium )
	:m_usedMedium( _medium )
{
}

template < class _RequestResponses >
_RequestResponses
 CRequestHandler< _RequestResponses >::getResponse( CRequest* _request ) const
{
	if( m_processedRequests.find( _request ) != m_processedRequests.end() )
		return m_processedRequests.find( _request )->second;
}

template < class _RequestResponses >
void
 CRequestHandler< _RequestResponses >::deleteRequest( CRequest* )
{

}

template < class _RequestResponses >
bool
 CRequestHandler< _RequestResponses >::isProcessed( CRequest* _request ) const
{
	if ( m_processedRequests.find( _request ) != m_processedRequests.end() )
		return true;

	return false;
}

template < class _RequestResponses >
bool
 CRequestHandler< _RequestResponses >::setRequest( CRequest* _request )
{
	m_newRequest.push_back( _request );
}

template < class _RequestResponses >
void
 CRequestHandler< _RequestResponses >::runRequests()
{
	BOOST_FOREACH( CRequest* request, m_newRequest )
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

		m_usedMedium->getResponse(requestResponses);

		// this i thing is temporary
		int i = 0;
		BOOST_FOREACH( _RequestResponses const & response, requestResponses )
		{
			m_processedRequests.insert( std::make_pair( m_newRequest[i++], response ) );
		}
		m_newRequest.clear();
	}
	 catch (CMediumException & _mediumException)
	{
// maybe  here pass global errors  like  problems  with  network
// pass it here  but keep in mind that at least for now every single  action is responsible  for handling errors

		BOOST_FOREACH( CRequest* request, m_newRequest )
		{
			m_processedRequests.insert( std::make_pair( request, CSystemError( _mediumException.m_error ) ) );
		}
		m_newRequest.clear();
	}

}

}

#endif
