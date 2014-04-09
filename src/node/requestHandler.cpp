// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "requestHandler.h"
#include "tracker/nodeMessages.h"
#include "medium.h"
#include "sendInfoRequestAction.h"
#include "support.h"
namespace node
{

CRequestHandler::CRequestHandler( CMedium * _medium )
	:m_usedMedium( _medium )
{
}

RequestRespond
CRequestHandler::getRespond( CRequest* _request ) const
{
	assert( m_processedRequests.find( _request ) != m_processedRequests.end() );
	return m_processedRequests.find( _request )->second;
}

void
CRequestHandler::deleteRequest( CRequest* )
{

}

bool
CRequestHandler::isProcessed( CRequest* _request ) const
{
	if ( m_pendingRequest.find( _request ) != m_pendingRequest.end() )
		return true;

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
		m_usedMedium->add( request );
	}

	m_usedMedium->flush();
}

void
CRequestHandler::readLoop()
{
	while(!m_usedMedium->serviced());

	CCommunicationBuffer response;
	m_usedMedium->getResponse(response);

	CBufferAsStream stream(
		  (char*)response.m_buffer
		, response.m_usedSize
		, SER_DISK
		, CLIENT_VERSION);

	unsigned int counter = 0;

	while( !stream.eof() )
	{
		int messageType;

		stream >> messageType;

		if ( messageType == self::CServerMessageType::ReferenceToken )
		{
			uint256 token;
			stream >> token;
			m_pendingRequest.insert( std::make_pair( m_newRequest[counter], token ) );

		}
		else if ( messageType == self::CServerMessageType::TransactionStatus )
		{
			int status;
			stream >> status;
			//m_processedRequests.insert( std::make_pair( m_newRequest[counter], CTransactionStatus( (self::TransactionsStatus::Enum )status ) ) );

		}
		else if ( messageType == self::CServerMessageType::MonitorInfo )
		{

		}
		else if ( messageType == self::CServerMessageType::TrackerInfo )
		{
			 CTrackerStats trackerInfo;
			readTrackerInfo( stream, trackerInfo, TrackerDescription );
			m_processedRequests.insert( std::make_pair( m_newRequest[counter], trackerInfo ) );

		}
		else if ( messageType == self::CServerMessageType::RequestSatatus )
		{

		}
		else
		{
			throw;
		}
		counter++;
	}
	m_newRequest.clear();

}


}