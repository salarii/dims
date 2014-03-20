#include "networkClient.h"

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
 
 
int CNetworkClient::waitForInput( QTcpSocket *socket )
{
	int bytesAvail = -1;

	while ( socket->state() == QAbstractSocket::ConnectedState && getRunThread() && bytesAvail < 0 )
	{
		if (socket->waitForReadyRead( m_timeout ))
		{
			bytesAvail = socket->bytesAvailable();
		}
		else
		{
			Sleep( 50 );
		}
	}

	return bytesAvail;
}
 
unsigned int
CNetworkClient::read(QTcpSocket *socket )
{
	int bytesAvail = waitForInput( socket );
	int bytesRead = 0;

	if (bytesAvail > 0)
	{
		int cnt = 0;
		bool endOfLine = false;
		bool endOfStream = false;

		while ( bytesRead < bytesAvail && (!endOfLine) && (!endOfStream) )
		{
			bytesRead += socket->read( m_inBuffor.m_buffer + bytesRead, MaxBufferSize - bytesRead );
			if ( MaxBufferSize == bytesRead )
				return 0; // this is  error so handle it as such  at some point

		}
	}
	return bytesRead;
}
 
 
void CNetworkClient::write(QTcpSocket *socket)
{
	if (m_outBuffor.m_usedSize > 0)
	{
		socket->write( m_outBuffor.m_buffer, m_outBuffor.m_usedSize );

		if (! socket->waitForBytesWritten())
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
	QTcpSocket socket;
	QHostAddress hostAddr( m_ip );
	socket.connectToHost( hostAddr, m_port );

	if ( socket.waitForConnected( m_timeout ) )
	{
		QHostAddress hostAddr = socket.localAddress();
		QString addr = "";
		if (hostAddr != QHostAddress::Null)
		{
			addr = hostAddr.toString();
		}

		if (addr.length() > 0)
		{
			//this  could be  somewhere in  gui
			//printf(" on address %s:d", addr.toAscii().data(), socket.localPort() );
		}
		
		// here send bytes to server
		while (socket.state() == QAbstractSocket::ConnectedState )
		{
			/*
			QString line( mStrings[ix] );
			writeLine(&socket, line);
			QString echoedLine = readLine( &socket );

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

void
CNetworkClient::send( CCommunicationBuffer const & _inBuffor )
{
	m_inBuffor = _inBuffor;
}

}