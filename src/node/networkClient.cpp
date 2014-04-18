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
    , m_connectionInfo( NoActivity )
{
	m_socket = new QTcpSocket(this);

	m_pushStream = new CBufferAsStream( (char*)m_pushBuffer.m_buffer, MaxBufferSize, SER_DISK, CLIENT_VERSION);
	m_pushStream->SetPos(0);
    startThread();
}
 
int CNetworkClient::waitForInput()
{
	int bytesAvail = -1;

	while ( m_socket->state() == QAbstractSocket::ConnectedState && m_connectionInfo == Processing && bytesAvail < 0 )
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
	m_pullBuffer.m_usedSize = 0;

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
	start();
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
		if ( m_connectionInfo == Processing )
        {
            QHostAddress hostAddr( m_ip );
            m_socket->connectToHost( hostAddr, m_port );
            if ( m_socket->waitForConnected( m_timeout ) )
            {
                write();
                read();
                m_connectionInfo = Processed;
            }
            else
            {
                m_connectionInfo = ServiceDenial;
            }
            m_socket->disconnectFromHost();
        }
    }
}

bool
CNetworkClient::serviced() const throw(CMediumException)
{
    if (m_connectionInfo == Processing )
        return false;
    else if ( m_connectionInfo == Processed )
        return true;
    else if ( m_connectionInfo == ServiceDenial )
        throw CMediumException(ErrorType::ServiceDenial );

    return false;
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
    if ( m_connectionInfo != NoActivity && m_connectionInfo != Processed )
        return false;

     m_connectionInfo = Processing;
}

bool
CNetworkClient::getResponse( common::CCommunicationBuffer & _outBuffor ) const
{
	_outBuffor = m_pullBuffer;
}

CNetworkClient::~CNetworkClient()
{
    // this is  not  right,  invent clean  way  to  terminate
    terminate();
    delete m_pushStream;
}

}
