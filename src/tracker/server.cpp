// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnectionFactory.h"

#include "server.h"

#include "serialize.h"

#include "common/nodeMessages.h"
#include "clientRequestsManager.h"

#include "common/ratcoinParams.h"
#include "common/support.h"

using namespace common;

/*
there is no  synchronization  fix  it
*/
namespace tracker
{

class CHandleResponseVisitor : public boost::static_visitor< void >
{
public:
	CHandleResponseVisitor( CBufferAsStream * _pushStream ):m_pushStream( _pushStream ){};

	void operator()( CAvailableCoins const & _availableCoins ) const
	{
		common::serializeEnum( *m_pushStream, CMainRequestType::BalanceInfoReq );
		*m_pushStream << _availableCoins;
	}

	void operator()( CDummy const & _dummy ) const
	{
		common::serializeEnum( *m_pushStream, CMainRequestType::ContinueReq );
		*m_pushStream << _dummy;
	}

	void operator()( CTrackerSpecificStats const & _trackerSpecificStats ) const
	{
		common::serializeEnum( *m_pushStream, CMainRequestType::TrackerInfoReq );
		*m_pushStream << _trackerSpecificStats;
	}

	void operator()( common::CClientNetworkInfoResult const & _networkInfo ) const
	{
		common::serializeEnum( *m_pushStream, CMainRequestType::NetworkInfoReq );
		*m_pushStream << _networkInfo;
	}
private:
	CBufferAsStream * const m_pushStream;

};


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
	m_clientRequestManager = CClientRequestsManager::getInstance();
}

void
CTcpServerConnection::run()
{
	Poco::Timespan timeOut(10,0);

	if (socket().poll(timeOut,Poco::Net::Socket::SELECT_READ) == false)
	{
		throw server_error(std::string( "TIMEOUT!" ));
	}
	else
	{

		try
		{
			m_pullBuffer.m_usedSize = socket().receiveBytes( m_pullBuffer.m_buffer, MaxBufferSize );
		}
		catch (Poco::Exception& exc) {
			//Handle your network errors.
			throw server_error(std::string( "Network error:" ) + exc.displayText() );
		}

		handleIncommingBuffor();
		int bytes;
		try
		{
			bytes = socket().sendBytes( m_pushBuffer.m_buffer, m_pushBuffer.m_usedSize );
		}
		catch (Poco::Exception& exc)
		{
			//Handle your network errors.
			throw server_error(std::string( "Network error:" ) + exc.displayText() );
		}
	}
	socket().close();

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


// annoying inconsistency below
bool
CTcpServerConnection::handleIncommingBuffor()
{
//MessageStartChars const & messageStart = m_networkParams->MessageStart();

//	if ( !checkSignature( stream ) )
//		return false;
//	writeSignature( outStream );
	CBufferAsStream pullStream(
		(char*)m_pullBuffer.m_buffer
		, m_pullBuffer.m_usedSize
		, SER_NETWORK
		, CLIENT_VERSION);

	CBufferAsStream pushStream(
		(char*)m_pushBuffer.m_buffer
		, MaxBufferSize
		, SER_NETWORK
		, CLIENT_VERSION
		, (uint64_t &)m_pushBuffer.m_usedSize
		);


	while( !pullStream.eof() )
	{
		int messageType;

		pullStream >> messageType;

		CTransaction transaction;
		if( messageType == CMainRequestType::Transaction )
		{
			pullStream >> transaction;
			common::serializeEnum( pushStream, CMainRequestType::ContinueReq );
			uint256 token = transaction.GetHash();
			m_clientRequestManager->addRequest( transaction, token );
			pushStream << token;
		}
		else if ( messageType == CMainRequestType::TrackerInfoReq )
		{
			common::serializeEnum( pushStream, CMainRequestType::ContinueReq );
			uint256 token = m_clientRequestManager->addRequest( CTrackerStatsReq() );
			pushStream << token;
		}
		else if ( messageType == CMainRequestType::MonitorInfoReq )
		{

		}
		else if ( messageType == CMainRequestType::TransactionStatusReq )
		{

		}
		else if ( messageType == CMainRequestType::BalanceInfoReq )
		{
			std::string address;
			pullStream >> address;
			common::serializeEnum( pushStream, CMainRequestType::ContinueReq );
			uint256 token = m_clientRequestManager->addRequest( address );
			pushStream << token;
		}
		else if ( messageType == CMainRequestType::ContinueReq )
		{
			uint256 token;
			pullStream >> token;
			ClientResponse clientResponse = m_clientRequestManager->getResponse( token );
			boost::apply_visitor( CHandleResponseVisitor( &pushStream ), clientResponse );
		}
		else if ( messageType == CMainRequestType::NetworkInfoReq )
		{

			uint256 token = m_clientRequestManager->addRequest( CNetworkInfoReq() );
			common::serializeEnum( pushStream, CMainRequestType::ContinueReq );
			pushStream << token;
		}
		else
		{
			return false;
		}
	}

}

void runServer()
{
	Poco::Net::TCPServer * server = new Poco::Net::TCPServer(
												  new Poco::Net::TCPServerConnectionFactoryImpl<CTcpServerConnection>()
												, Poco::Net::ServerSocket(common::ratcoinParams().getDefaultClientPort())
												);

	server->start();
}

}
