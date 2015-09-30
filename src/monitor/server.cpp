// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Poco/Net/TCPServer.h"
#include "Poco/Net/TCPServerConnectionFactory.h"

#include "server.h"

#include "serialize.h"

#include "common/nodeMessages.h"
#include "clientRequestsManager.h"

#include "common/dimsParams.h"
#include "common/support.h"

using namespace common;

/*
there is no  synchronization  fix  it
*/
namespace monitor
{

class CHandleResponseVisitor : public boost::static_visitor< void >
{
public:
	CHandleResponseVisitor( CBufferAsStream * _pushStream, uint256 const & _token ):m_pushStream( _pushStream ),m_token(_token){};

	void operator()( CAvailableCoinsData const & _availableCoins ) const
	{
		std::vector< unsigned char > payload;
		common::createPayload( _availableCoins, payload );

		CClientMessage message( CMainRequestType::BalanceInfoReq, payload, m_token );
		*m_pushStream << message;
	}

	void operator()( CTrackerSpecificStats const & _trackerSpecificStats ) const
	{
		std::vector< unsigned char > payload;
		common::createPayload( _trackerSpecificStats, payload );

		CClientMessage message( CMainRequestType::TrackerInfoReq, payload, m_token );
		*m_pushStream << message;
	}

	void operator()( CMonitorData const & _monitorData ) const
	{
		std::vector< unsigned char > payload;
		common::createPayload( _monitorData, payload );

		CClientMessage message( CMainRequestType::MonitorInfoReq, payload, m_token );
		*m_pushStream << message;
	}

	void operator()( common::CClientNetworkInfoResult const & _networkInfo ) const
	{
		std::vector< unsigned char > payload;
		common::createPayload( _networkInfo, payload );

		CClientMessage message( CMainRequestType::NetworkInfoReq, payload, m_token );
		*m_pushStream << message;
	}

	void operator()( common::CTransactionAck const & _transactionAck ) const
	{
		std::vector< unsigned char > payload;
		common::createPayload( _transactionAck, payload );

		CClientMessage message( CMainRequestType::Transaction, payload, m_token );
		*m_pushStream << message;
	}

	void operator()( common::CTransactionStatusResponse const & _transactionStatus ) const
	{
		std::vector< unsigned char > payload;
		common::createPayload( _transactionStatus, payload );

		CClientMessage message( CMainRequestType::TransactionStatusReq, payload, m_token );
		*m_pushStream << message;
	}
private:
	CBufferAsStream * const m_pushStream;

	uint256 m_token;
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
		, SER_NETWORK
		, CLIENT_VERSION)*/
{
}

void
CTcpServerConnection::run()
{
	Poco::Timespan timeOut(10,0);
	while( 1 )
	{
		if (socket().poll(timeOut,Poco::Net::Socket::SELECT_READ ))
		{
			try
			{
				m_pullBuffer.m_usedSize = socket().receiveBytes( m_pullBuffer.m_buffer, MaxBufferSize );
			}
			catch (Poco::Exception& exc) {
				//Handle your network errors.
				throw server_error(std::string( "Network error:" ) + exc.displayText() );
			}

		}
		handleIncommingBuffor();

		if (socket().poll(timeOut, Poco::Net::Socket::SELECT_WRITE))
		{
			try
			{
				socket().sendBytes( m_pushBuffer.m_buffer, m_pushBuffer.m_usedSize );
			}
			catch (Poco::Exception& exc)
			{
				//Handle your network errors.
				throw server_error(std::string( "Network error:" ) + exc.displayText() );
			}
		}
		if ( m_tokens.empty() )
		{
			socket().close();
			break;
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
		common::CClientMessage clientMessage;

		pullStream >> clientMessage;

		CTransaction transaction;

		if ( clientMessage.m_header.m_payloadKind == CMainRequestType::TrackerInfoReq )
		{
		}
		else if ( clientMessage.m_header.m_payloadKind == CMainRequestType::MonitorInfoReq )
		{
			CClientRequestsManager::getInstance()->addRequest( CMonitorInfoReq(), clientMessage.m_header.m_id );
			m_tokens.insert( clientMessage.m_header.m_id );
		}
		else if ( clientMessage.m_header.m_payloadKind == CMainRequestType::NetworkInfoReq )
		{
			CClientRequestsManager::getInstance()->addRequest( CNetworkInfoReq(), clientMessage.m_header.m_id );
			m_tokens.insert( clientMessage.m_header.m_id );
		}
		else
		{
			return false;
		}
	}

	std::list< uint256 > toRemove;
	ClientResponse clientResponse;
	BOOST_FOREACH( uint256 const & token, m_tokens )
	{
		if ( CClientRequestsManager::getInstance()->getResponse( token, clientResponse ) )
		{
			boost::apply_visitor( CHandleResponseVisitor( &pushStream, token ), clientResponse );
			toRemove.push_back( token );
		}
	}

	BOOST_FOREACH( uint256 const & token, toRemove )
	{
		m_tokens.erase( token );
	}

	m_pullBuffer.clear();
	return true;
}

void runServer()
{
	Poco::Net::TCPServer * server = new Poco::Net::TCPServer(
												  new Poco::Net::TCPServerConnectionFactoryImpl<CTcpServerConnection>()
												, Poco::Net::ServerSocket(common::dimsParams().getDefaultClientPort())
												);

	server->start();
}

}
