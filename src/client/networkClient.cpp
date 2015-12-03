// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "networkClient.h"
#include "serialize.h"
#include "version.h"

#include "qt/sendcoinsdialog.h"

#include "client/sendBalanceInfoAction.h"
#include "client/sendTransactionAction.h"
#include "client/sendInfoRequestAction.h"
#include "client/requests.h"
#include "client/trackerLocalRanking.h"

#include "common/clientProtocol.h"
#include "common/requestHandler.h"
#include "common/support.h"
#include "common/requests.h"

#include"wallet.h"

#include <QHostAddress>

namespace client
{

unsigned const
CNetworkClient::m_timeout = 5000;

CNetworkClient::CNetworkClient(QString const &_ipAddr, ushort const _port )
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
			return bytesAvail;
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
		qint64 written = m_socket->write( m_pushBuffer.m_buffer, m_pushBuffer.m_usedSize );

		if (! m_socket->waitForBytesWritten())
		{
			// error handle it somehow
		}

		m_pushBuffer.m_usedSize = 0;
	}
}

common::CRequest*
CNetworkClient::takeMatching( uint256 const & _token )
{
	std::map< uint256, common::CRequest* >::iterator iterator
			= m_matching.find( _token );

	common::CRequest* request;

	if( iterator != m_matching.end() )
	{
		request = iterator->second;
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
CNetworkClient::add( common::CRequest const * _request )
{
}

void
CNetworkClient::add( common::CSendClientMessageRequest const * _request )
{
	QMutexLocker lock( &m_mutex );

	*m_pushStream << _request->getClientMessage();

	m_matching.insert( std::make_pair( _request->getId(), ( common::CRequest* )_request ) );

	m_workingRequest.push_back( ( common::CRequest* )_request );
}

void
CNetworkClient::add( client::CCreateTransactionRequest const * _request )
{
	QMutexLocker lock( &m_mutex );

	CPubKey trackerKey;

	if ( !CTrackerLocalRanking::getInstance()->getNodeKey( m_ip.toStdString(), trackerKey ) )
	{
		assert(!"problem");
		return;
	}

	common::CTrackerStats trackerStats;

	if ( !CTrackerLocalRanking::getInstance()->getTrackerStats( trackerKey.GetID(), trackerStats ) )
	{
		assert(!"problem");
		return;
	}

	CWalletTx tx;
	std::string failReason;
	if ( !CWallet::getInstance()->CreateTransaction( _request->m_outputs, _request->m_sendCoins, trackerStats.m_key, trackerStats.m_price, tx, failReason ) )
	{
		// notify user  about  this
		return;
	}

	SendSentinel.m_userResponded = false;

	emit SendSentinel.requestAcceptance( 10, 10 );

	while( !SendSentinel.m_userResponded )
	{
		MilliSleep(100);
	}

	if ( !SendSentinel.m_userResponse )
		return;

	CTransaction * transaction = &tx;

	std::vector< unsigned char > payload;
	common::createPayload( common::CClientTransactionSend(*transaction), payload );
	common::CClientMessage clientMessage(
				common::CMainRequestType::Transaction
				, payload
				, transaction->GetHash() );

		*m_pushStream << clientMessage;

	m_matching.insert( std::make_pair( transaction->GetHash(), ( common::CRequest* )_request ) );

	m_workingRequest.push_back( ( common::CRequest* )_request );
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
CNetworkClient::getResponseAndClear( std::multimap< common::CRequest const*, common::DimsResponse > & _requestResponse )
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
							, common::CClientMessageResponse( clientMessage, common::convertToInt(this), m_ip.toStdString() ) ) );
			m_matching.erase( clientMessage.m_header.m_id );

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
