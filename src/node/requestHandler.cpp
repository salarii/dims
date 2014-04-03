// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "requestHandler.h"
#include "tracker/nodeMessages.h"

namespace node
{

CRequestHandler::CRequestHandler( CNetworkClient * _networkClient )
	:m_networkClient( _networkClient )
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
	CCommunicationBuffer inBuffor;

	CBufferAsStream stream(
		(char*)inBuffor.m_buffer
		, inBuffor.m_usedSize
		, SER_DISK
		, CLIENT_VERSION);

	BOOST_FOREACH( CRequest* request, m_newRequest )
	{
//		m_networkClient
//			request

		request->serialize( stream );

	}

	m_networkClient->send( inBuffor );

	readLoop();
}

void
CRequestHandler::readLoop()
{
	while(!m_networkClient->serviced());

	CCommunicationBuffer response;
	m_networkClient->getResponse(response);

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