// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "requestHandler.h"
#include "tracker/nodeMessages.h"
#include "medium.h"
#include "support.h"

#include "common/communicationBuffer.h"

namespace common
{

CRequestHandler::CRequestHandler( CMedium * _medium )
	:m_usedMedium( _medium )
{
}

RequestResponse
CRequestHandler::getRespond( CRequest* _request ) const
{
	if( m_processedRequests.find( _request ) != m_processedRequests.end() )
		return m_processedRequests.find( _request )->second;
}

void
CRequestHandler::deleteRequest( CRequest* )
{

}

bool
CRequestHandler::isProcessed( CRequest* _request ) const
{
	if ( m_processedRequests.find( _request ) != m_processedRequests.end() )
		return true;
	
	return false;
}

bool 
CRequestHandler::setRequest( CRequest* _request )
{
	m_newRequest.push_back( _request );
}


void
CRequestHandler::runRequests()
{
	BOOST_FOREACH( CRequest* request, m_newRequest )
	{
		request->accept( m_usedMedium );
	}

	m_usedMedium->flush();
}

void
CRequestHandler::readLoop()
{
    try
    {
        while(!m_usedMedium->serviced());

		std::vector< RequestResponse > requestResponses;

		m_usedMedium->getResponse(requestResponses);

		// this i thing is temporary
		int i = 0;
		BOOST_FOREACH( RequestResponse const & response, requestResponses )
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
