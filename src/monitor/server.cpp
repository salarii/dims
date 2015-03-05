// Copyright (c) 2014-2015 Dims dev-team
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

	void operator()( CAvailableCoins const & _availableCoins ) const
	{
		common::serializeEnum( *m_pushStream, CMainRequestType::BalanceInfoReq );
		*m_pushStream << m_token;
		*m_pushStream << _availableCoins;
	}

	void operator()( CTrackerSpecificStats const & _trackerSpecificStats ) const
	{
		common::serializeEnum( *m_pushStream, CMainRequestType::TrackerInfoReq );
		*m_pushStream << m_token;
		*m_pushStream << _trackerSpecificStats;
	}

	void operator()( CMonitorData const & _monitorData ) const
	{
		common::serializeEnum( *m_pushStream, CMainRequestType::MonitorInfoReq );
		*m_pushStream << m_token;
		*m_pushStream << _monitorData;
	}

	void operator()( common::CClientNetworkInfoResult const & _networkInfo ) const
	{
		common::serializeEnum( *m_pushStream, CMainRequestType::NetworkInfoReq );
		*m_pushStream << m_token;
		*m_pushStream << _networkInfo;
	}

	void operator()( common::CTransactionAck const & _transactionAck ) const
	{
		common::serializeEnum( *m_pushStream, CMainRequestType::Transaction );
		*m_pushStream << m_token;
		*m_pushStream << _transactionAck;
	}

	void operator()( common::CTransactionStatusResponse const & _transactionStatus ) const
	{
		common::serializeEnum( *m_pushStream, CMainRequestType::TransactionStatusReq );
		*m_pushStream << m_token;
		*m_pushStream << _transactionStatus;
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
		int messageType;

		pullStream >> messageType;

		CTransaction transaction;

		if ( messageType == CMainRequestType::TrackerInfoReq )
		{
		}
		else if ( messageType == CMainRequestType::MonitorInfoReq )
		{
			uint256 token = CClientRequestsManager::getInstance()->addRequest( CMonitorInfoReq() );
			common::serializeEnum( pushStream, CMainRequestType::ContinueReq );
			pushStream << token;
			m_tokens.insert( token );
		}
		else if ( messageType == CMainRequestType::NetworkInfoReq )
		{

			uint256 token = CClientRequestsManager::getInstance()->addRequest( CNetworkInfoReq() );
			common::serializeEnum( pushStream, CMainRequestType::ContinueReq );
			pushStream << token;
			m_tokens.insert( token );
		}
		else
		{
			return false;
		}
	}

	std::list< uint256 > toRemove;
	ClientResponse clientResponse;
	BOOST_FOREACH( uint256 const & token,m_tokens )
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
