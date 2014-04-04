// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "networkClient.h"
#include "serialize.h"
#include "version.h"
#include "requestHandler.h"

#include <QHostAddress>

namespace node
{

unsigned const
CNetworkClient::m_timeout = 5000;

CNetworkClient::CNetworkClient(QString const  &_ipAddr, ushort const _port )
	: m_ip( _ipAddr )
	, m_port( _port )
{
	mRunThread = false;

	m_pushStream = new CBufferAsStream( (char*)m_pushBuffer.m_buffer, MaxBufferSize, SER_DISK, CLIENT_VERSION);
	m_pushStream->SetPos(0);
}


void CNetworkClient::setRunThread(bool newVal )
{
	QMutexLocker lock( &m_mutex );
	mRunThread = newVal;
}
 
bool CNetworkClient::getRunThread()
{
	QMutexLocker lock( &m_mutex );
	return mRunThread;
}
 
 
int CNetworkClient::waitForInput()
{
	int bytesAvail = -1;

	while ( m_socket->state() == QAbstractSocket::ConnectedState && getRunThread() && bytesAvail < 0 )
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
	int bytesAvail = waitForInput();
	int bytesRead = 0;

	if (bytesAvail > 0)
	{
		int cnt = 0;
		bool endOfLine = false;
		bool endOfStream = false;

		while ( bytesRead < bytesAvail && (!endOfLine) && (!endOfStream) )
		{
			bytesRead += m_socket->read( m_pullBuffer.m_buffer + bytesRead, MaxBufferSize - bytesRead );
			if ( MaxBufferSize == bytesRead )
				return 0; // this is  error so handle it as such  at some point

		}
	}
	return bytesRead;
}
 
 
void CNetworkClient::write()
{
	if (m_pushBuffer.m_usedSize > 0)
	{
		m_socket->write( m_pushBuffer.m_buffer, m_pushBuffer.m_usedSize );

		if (! m_socket->waitForBytesWritten())
		{
			// error handle it somehow
		}
	}
}
 
void CNetworkClient::startThread()
{
	setRunThread( true );
	start();
}
 
void CNetworkClient::stopThread()
{
	setRunThread( false );
}

void CNetworkClient::run()
{
	QHostAddress hostAddr( m_ip );
	m_socket->connectToHost( hostAddr, m_port );

	if ( m_socket->waitForConnected( m_timeout ) )
	{
		QHostAddress hostAddr = m_socket->localAddress();
		QString addr = "";
		if (hostAddr != QHostAddress::Null)
		{
			addr = hostAddr.toString();
		}

		if (addr.length() > 0)
		{
			//this  could be  somewhere in  gui
			//printf(" on address %s:d", addr.toAscii().data(), m_socket.localPort() );
		}
		
		// here send bytes to server
		while (m_socket->state() == QAbstractSocket::ConnectedState )
		{
			/*
			QString line( mStrings[ix] );
			writeLine(&m_socket, line);
			QString echoedLine = readLine( &m_socket );

			if (echoedLine.length() > 0)
			{
				if (line != echoedLine)
				{
				printf("line and echoed line doesn't match\n");
				}
				else
				{
				printf("%s\n", line.toAscii().data() );
				}
			}
			ix++;*/
		} 
	}
	else
	{
		printf("Client socket failed to connect\n");
	}
}

bool
CNetworkClient::serviced() const
{
	return true;
}


void 
CNetworkClient::add( CRequest const * _request )
{
	_request->serialize( *m_pushStream );
}

bool
CNetworkClient::flush()
{
	m_pushBuffer.m_usedSize = m_pushStream->GetPos();
	write();
	m_pushStream->SetPos( 0 );
	read();
}

bool
CNetworkClient::getResponse( CCommunicationBuffer & _outBuffor ) const
{
	_outBuffor = m_pullBuffer;
}

}