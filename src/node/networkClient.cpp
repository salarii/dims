// Copyright (c) 2014 Ratcoin dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "networkClient.h"
#include "serialize.h"
#include "version.h"
#include "common/requestHandler.h"
#include "sendBalanceInfoAction.h"
#include "sendTransactionAction.h"
#include "sendInfoRequestAction.h"
#include "helper.h"

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


void CNetworkClient::run()
{
    while(1)
    {

		if ( m_connectionInfo == Processing )
        {
			QMutexLocker lock( &m_mutex );
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
CNetworkClient::serviced() const throw(common::CMediumException)
{
    if (m_connectionInfo == Processing )
        return false;
    else if ( m_connectionInfo == Processed )
        return true;
    else if ( m_connectionInfo == ServiceDenial )
		throw common::CMediumException(common::ErrorType::ServiceDenial );

    return false;
}


void 
CNetworkClient::add( common::CRequest< NodeResponses > const * _request )
{
}

void
CNetworkClient::add( CBalanceRequest const * _request )
{
	QMutexLocker lock( &m_mutex );
	serializeEnum(*m_pushStream, common::CMainRequestType::BalanceInfoReq );
	*m_pushStream << _request->m_address;
}

void
CNetworkClient::add( CTransactionSendRequest const * _request )
{
	QMutexLocker lock( &m_mutex );
	serializeEnum(*m_pushStream, common::CMainRequestType::Transaction );
	*m_pushStream  << _request->m_transaction;
}


bool
CNetworkClient::flush()
{
	QMutexLocker lock( &m_mutex );
	m_pushBuffer.m_usedSize = m_pushStream->GetPos();

	if ( !m_pushBuffer.m_usedSize )
		return true;
	m_pushStream->SetPos( 0 );

    if ( m_connectionInfo != NoActivity && m_connectionInfo != Processed )
        return false;

     m_connectionInfo = Processing;
}

bool
CNetworkClient::getResponse( std::vector< NodeResponses > & _requestResponse ) const
{
	QMutexLocker lock( &m_mutex );
	CBufferAsStream stream(
		  (char*)m_pullBuffer.m_buffer
		, m_pullBuffer.m_usedSize
		, SER_DISK
		, CLIENT_VERSION);

	while( !stream.eof() )
	{
		int messageType;

		stream >> messageType;

		if ( messageType == common::CMainRequestType::ContinueReq )
		{
			uint256 token;
			stream >> token;
			_requestResponse.push_back( common::CPending(token) );

		}
		else if ( messageType == common::CMainRequestType::TransactionStatusReq )
		{
			int status;
			stream >> status;
			//m_processedRequests.insert( std::make_pair( m_newRequest[counter], CTransactionStatus( (TransactionsStatus::Enum )status ) ) );

		}
		else if ( messageType == common::CMainRequestType::MonitorInfoReq )
		{

		}
		else if ( messageType == common::CMainRequestType::TrackerInfoReq )
		{
			common::CTrackerStats trackerInfo;

			readTrackerInfo( stream, trackerInfo, TrackerDescription );
			_requestResponse.push_back( trackerInfo );

		}
		else if ( messageType == common::CMainRequestType::RequestSatatusReq )
		{
		}
		else if ( messageType == common::CMainRequestType::BalanceInfoReq )
		{
			common::CAvailableCoins availableCoins;
			stream >> availableCoins;
			_requestResponse.push_back( availableCoins );
		}
		else
		{
			throw;
		}
	}


}

CNetworkClient::~CNetworkClient()
{
    // this is  not  right,  invent clean  way  to  terminate
    terminate();
    delete m_pushStream;
}

}
