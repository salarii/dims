// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "requestHandler.h"
#include "tracker/nodeMessages.h"
#include "medium.h"

namespace node
{

CRequestHandler::CRequestHandler( CMedium * _medium )
	:m_usedMedium( _medium )
{
}

RequestRespond
CRequestHandler::getRespond( CRequest* _request ) const
{

}

bool
CRequestHandler::isProcessed( CRequest* _request ) const
{

}

bool 
CRequestHandler::setRequest( CRequest* _request ) const
{

}


void
CRequestHandler::runRequests()
{
	BOOST_FOREACH( CRequest* request, m_newRequest )
	{
		m_usedMedium->add( request );
	}

	m_usedMedium->flush();

	readLoop();
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

		counter++;

		uint256 token;
		int status;

		switch( (self::CServerMessageType::Enum)messageType )
		{
		case self::CServerMessageType::ReferenceToken:
			stream >> token;
			m_pendingRequest.insert( std::make_pair( m_newRequest[counter], token ) );
			break;
		case self::CServerMessageType::TransactionStatus:
			stream >> status;
			//m_processedRequests.insert( std::make_pair( m_newRequest[counter], CTransactionStatus( (self::TransactionsStatus::Enum )status ) ) );
			break;
		case self::CServerMessageType::MonitorInfo:
			break;
		case self::CServerMessageType::Trackernfo:
			break;
		case self::CServerMessageType::RequestSatatus:
			break;
		default:
			return;
		}
	}
}


}