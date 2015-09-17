// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "networkClient.h"
#include "serialize.h"
#include "version.h"
#include "client/sendBalanceInfoAction.h"
#include "client/sendTransactionAction.h"
#include "client/sendInfoRequestAction.h"
#include "client/clientRequests.h"

#include "common/clientProtocol.h"
#include "common/requestHandler.h"
#include "common/support.h"
#include "common/commonRequests.h"

#include <QHostAddress>

namespace client
{

unsigned const
CNetworkClient::m_timeout = 5000;

CNetworkClient::CNetworkClient(QString const  &_ipAddr, ushort const _port )
	: m_ip( _ipAddr )
	, m_port( _port )
	, m_sleepTime(100)
{
	m_socket = new QTcpSocket(this);

	m_pushStream = new CBufferAsStream( (char*)m_pushBuffer.m_buffer, MaxBufferSize, SER_NETWORK, CLIENT_VERSION);
	m_pushStream->SetPos(0);
	start();
}
unsigned int
CNetworkClient::waitForInput()
{
	unsigned int bytesAvail = 0;

	while( m_socket->state() == QAbstractSocket::ConnectedState && bytesAvail == 0 )
	{
		if (m_socket->waitForReadyRead( m_timeout ))
		{
			bytesAvail = m_socket->bytesAvailable();
		}
		else
		{
			QThread::msleep( 50 );
		}
	}
	return bytesAvail;
}

unsigned int
CNetworkClient::read()
{
	unsigned int bytesAvail = waitForInput();

	if (bytesAvail > 0)
	{
		bool endOfLine = false;
		bool endOfStream = false;

		if ( MaxBufferSize <= bytesAvail )
		{
			//throw error
			return 0; // this is  error so handle it as such  at some point
		}

		while ( m_pullBuffer.m_usedSize < bytesAvail && (!endOfLine) && (!endOfStream) )
		{
			m_pullBuffer.m_usedSize += m_socket->read( m_pullBuffer.m_buffer + m_pullBuffer.m_usedSize, MaxBufferSize - m_pullBuffer.m_usedSize );

		}
	}
	return m_pullBuffer.m_usedSize;
}


void
CNetworkClient::write()
{
	if (m_pushBuffer.m_usedSize > 0)
	{
		m_socket->write( m_pushBuffer.m_buffer, m_pushBuffer.m_usedSize );

		if (! m_socket->waitForBytesWritten())
		{
			// error handle it somehow
		}

		m_pushBuffer.m_usedSize = 0;
	}
}

common::CRequest< common::CClientTypes >*
CNetworkClient::takeMatching( uint256 const & _token )
{
	std::map< uint256, common::CRequest< common::CClientTypes >* >::iterator iterator
			= m_matching.find( _token );

	common::CRequest< common::CClientTypes >* request;

	if( iterator != m_matching.end() )
	{
		request = iterator->second;
		m_matching.erase( iterator );
	}
	else
	{
		assert( !"can't be here" );
		return 0;
	}
	return request;
}

bool
CNetworkClient::dropConnection() const
{
	return m_matching.empty() && m_workingRequest.empty();
}

void CNetworkClient::run()
{
	while(1)
	{
		{
			QMutexLocker lock( &m_mutex );
			QHostAddress hostAddr( m_ip );

			if ( !m_socket->isOpen () || !(m_socket->state() == QAbstractSocket::ConnectedState) )
			{
				m_socket->connectToHost( hostAddr, m_port );
				if ( !m_socket->waitForConnected( m_timeout ) )
				{
					assert( !"hadle this" );
				}
			}

			write();
			read();
		}
		MilliSleep( m_sleepTime );

		{
			// how to exit 100% safe, this  is not  the way to go for sure
			QMutexLocker lock( &m_mutex );
			if ( dropConnection() )
			{
				m_socket->disconnectFromHost();
				break;
			}
		}
	}
}

bool
CNetworkClient::serviced() const throw(common::CMediumException)
{
	return m_pullBuffer.m_usedSize > 0;
}

void
CNetworkClient::prepareMedium()
{
	if ( !isRunning() )
		start();
}

void 
CNetworkClient::add( common::CRequest< common::CClientTypes > const * _request )
{
}

void
CNetworkClient::add( common::CSendMessageRequest< common::CClientTypes > const * _request )
{
	QMutexLocker lock( &m_mutex );

	*m_pushStream <<
					 common::CClientMessage(
						 _request->getMessageKind()
						 , _request->getPayLoad()
						 , _request->getId() );

	m_matching.insert( std::make_pair( _request->getId(), ( common::CRequest< common::CClientTypes >* )_request ) );

	m_workingRequest.push_back( ( common::CRequest< common::CClientTypes >* )_request );
}

bool
CNetworkClient::flush()
{
	QMutexLocker lock( &m_mutex );
	m_pushBuffer.m_usedSize = m_pushStream->GetPos();

	if ( !m_pushBuffer.m_usedSize )
		return true;
	m_pushStream->SetPos( 0 );

	return true;
}

void
CNetworkClient::clearResponses()
{
	m_pullBuffer.m_usedSize = 0;
}

bool
CNetworkClient::getResponseAndClear( std::multimap< common::CRequest< common::CClientTypes >const*, ClientResponses > & _requestResponse )
{
	QMutexLocker lock( &m_mutex );
	CBufferAsStream stream(
				(char*)m_pullBuffer.m_buffer
				, m_pullBuffer.m_usedSize
				, SER_NETWORK
				, CLIENT_VERSION);

	uint256 token;

	while( !stream.eof() )
	{
		int messageType;

		common::CClientMessage clientMessage;
		try
		{
			stream >> clientMessage;

			assert( takeMatching( clientMessage.m_header.m_id ) );

			_requestResponse.insert(
						std::make_pair(
							takeMatching( clientMessage.m_header.m_id )
							, common::CClientMessageResponse( clientMessage, common::convertToInt(this) ) ) );
		}
		catch(...)
		{}
	}

	clearResponses();
	return true;
}

CNetworkClient::~CNetworkClient()
{
	//  is this right??, maybe invent some other  way  to  terminate?
	terminate();
	delete m_pushStream;
}

}
