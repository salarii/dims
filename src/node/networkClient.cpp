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
    , m_flushBuffor(false)
{
	mRunThread = false;

	m_pushStream = new CBufferAsStream( (char*)m_pushBuffer.m_buffer, MaxBufferSize, SER_DISK, CLIENT_VERSION);
	m_pushStream->SetPos(0);
    startThread();
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

/*
run will go instantly
whenever write come
do following
connect  send
receive
  disconnect
sign  serviced

*/
void CNetworkClient::run()
{
    while(1)
    {
        QMutexLocker lock( &m_writeMutex );
        if ( m_flushBuffor == true )
        {
            QHostAddress hostAddr( m_ip );
            m_socket->connectToHost( hostAddr, m_port );
            if ( m_socket->waitForConnected( m_timeout ) )
            {
                write();
                read();
            }
            else
            {
                printf("Client socket failed to connect\n");
            }

            m_socket->disconnectFromHost();
        }
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
	m_pushStream->SetPos( 0 );

    QMutexLocker lock( &m_writeMutex );
    if ( m_flushBuffor != false )
        return false;

    m_flushBuffor = true;
}

bool
CNetworkClient::getResponse( common::CCommunicationBuffer & _outBuffor ) const
{
	_outBuffor = m_pullBuffer;
}

CNetworkClient::~CNetworkClient()
{
    // this is  not  right,  invent clean  way  to  terminate
    stopThread();
    terminate();
    delete m_pushStream;
    delete m_socket;
}

}
