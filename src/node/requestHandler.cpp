#include "requestHandler.h"

namespace node
{

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
		m_networkClient
			request

			request->serialize( stream );

	}

	m_networkClient->send( CCommunicationBuffer _inBuffor );

	readLoop();
}

void
CRequestHandler::readLoop()
{
	while(!m_networkClient->serviced());

	CCommunicationBuffer response = m_networkClient->getResponse();

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

		switch( (CServerMessageType::Enum)messageType )
		{
		case CServerMessageType::ReferenceToken:
			uint256 token;
			stream >> token;
			m_pendingRequest.insert( make_pair( m_newRequest[counter], token ) );
			break;
		case CServerMessageType::TransactionStatus:
			TransactionsStatus::Enum status;
			stream >> status;
			m_processedRequests.insert( make_pair( m_newRequest[counter], status ) );
			break;
		case CServerMessageType::MonitorInfo:
			break;
		case CServerMessageType::Trackernfo:
			break;
		case CServerMessageType::RequestSatatus:
			break;
		default:
			return false;
		}
	}
}

}