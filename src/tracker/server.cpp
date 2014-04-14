// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnectionFactory.h"

#include "server.h"

#include "serialize.h"

#include "nodeMessages.h"

namespace tracker
{

class server_error : public std::runtime_error
{
public:
	explicit server_error(const std::string& str) : std::runtime_error(str) {}
};


CTcpServerConnection::CTcpServerConnection(Poco::Net::StreamSocket const & _serverConnection )
	: Poco::Net::TCPServerConnection( _serverConnection )
/*
	, pushStream(
		(char*)m_outgoingBuffer.m_buffer
		, MaxBufferSize
		, SER_DISK
		, CLIENT_VERSION)*/
{
}

void
CTcpServerConnection::run()
{
	bool isOpen = true;
	Poco::Timespan timeOut(10,0);
	while( isOpen )
	{
		if (socket().poll(timeOut,Poco::Net::Socket::SELECT_READ) == false){
			throw server_error(std::string( "TIMEOUT!" ));
		}
		else{
			int nBytes = -1;

			try 
			{
				nBytes = socket().receiveBytes( m_pullBuffer.m_buffer, MaxBufferSize );
			}
			catch (Poco::Exception& exc) {
				//Handle your network errors.
				throw server_error(std::string( "Network error:" ) + exc.displayText() );
				isOpen = false;
			}


			if (nBytes==0)
			{
				isOpen = false;
			}
			handleIncommingBuffor();
			
			// answare  to  incomming  connections 
		}
	}
}

void
CTcpServerConnection::writeSignature( CBufferAsStream & _stream )
{
/*	unsigned char signatureByte;
	for ( int i = 0 ; i < sizeof( MessageStartChars ); i++ )
	{
		_stream << signatureByte;

		if ( signatureByte != messageStart[ i ] )
			return false;
	}*/
}


bool
CTcpServerConnection::checkSignature( CBufferAsStream const & _stream )
{
/*	unsigned char signatureByte;
	for ( int i = 0 ; i < sizeof( MessageStartChars ); i++ )
	{
		_stream >> signatureByte;

		if ( signatureByte != messageStart[ i ] )
			return false;
	}*/
	return true;

}

bool
CTcpServerConnection::handleIncommingBuffor()
{
//MessageStartChars const & messageStart = m_networkParams->MessageStart();

//	if ( !checkSignature( stream ) )
//		return false;
//	writeSignature( outStream );
	CBufferAsStream pullStream(
		(char*)m_pullBuffer.m_buffer
		, MaxBufferSize
		, SER_DISK
		, CLIENT_VERSION);

	while( !pullStream.eof() )
	{
		int messageType;

		pullStream >> messageType;

		CTransaction transaction;
		switch( (CMainRequestType::Enum)messageType )
		{
			case CMainRequestType::Transaction:
				pullStream >> transaction;
				/*outStream << transaction.GetHash();
				m_validationManager->serviceTransaction( transaction );*/
				break;
			case CMainRequestType::TrackerInfoReq:
				break;
			case CMainRequestType::MonitorInfoReq:
				break;
			case CMainRequestType::TransactionStatusReq:
				break;
			case CMainRequestType::ContinueReq:
				break;
			default:
				return false;
		}
	}

}

void runServer()
{
	Poco::Net::TCPServer * server = new Poco::Net::TCPServer(
												  new Poco::Net::TCPServerConnectionFactoryImpl<CTcpServerConnection>()
												, Poco::Net::ServerSocket(1)
												);

	server->start();
}
/*
int sendBytes(
const void * buffer,
int length,
int flags = 0
);

*/
}