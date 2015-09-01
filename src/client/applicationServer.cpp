// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/filesystem.hpp>

#include "common/medium.h"
#include "common/support.h"
#include "common/actionHandler.h"

#include "appLib/messageType.h"
#include "appLib/paymentData.h"

#include "payLocalApplicationAction.h"
#include "applicationServer.h"
#include "clientRequests.h"
#include "clientFilters.h"

namespace client
{

CLocalSocket::CLocalSocket( QLocalSocket * _localSocket )
	:m_localSocket( _localSocket )
{
}

CLocalSocket::~CLocalSocket()
{
	size_t size = m_localSocket->bytesAvailable ();
	if ( !size )
		m_localSocket->close();
	else
		m_localSocket->deleteLater();
}

void
CLocalSocket::handleInput()
{
	QByteArray block = m_localSocket->readAll();

	CBufferAsStream stream(
		  (char*)block.data()
		, block.size()
		, SER_NETWORK
		, CLIENT_VERSION);

	int kind;

	stream >> kind;

	if ( kind == dims::CMessageKind::Expectations )
	{
		dims::CExpectationMessage expectationMessage;
		stream >> expectationMessage;

		common::CActionHandler< common::CClientTypes >::getInstance()->executeAction(
					new CPayLocalApplicationAction( common::convertToInt( m_localSocket ), expectationMessage.m_privateKey, expectationMessage.m_targetId, expectationMessage.m_value, expectationMessage.m_trackers, expectationMessage.m_monitors ) );

	}
//	else if()
	{

	}

}
/*
if(socket)
{
	if(socket->isOpen())
	{
		QByteArray block;
		QDataStream out(&block, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_0);
		out << sms;
		socket->write(block);
	}
}
*/
bool
CLocalSocket::serviced() const throw(common::CMediumException)
{
	return true;
}

bool
CLocalSocket::flush()
{
	return true;
}

void
CLocalSocket::clearResponses()
{
	m_nodeResponses.clear();
}

bool
CLocalSocket::getResponseAndClear( std::multimap< common::CRequest< common::CClientTypes >const*, ClientResponses > & _requestResponse )
{
	_requestResponse = m_nodeResponses;

	clearResponses();

	return true;
}

void
CLocalSocket::add( CErrorForAppPaymentProcessing const * _request )
{
	int messageCode = dims::CMessageKind::ErrorIndicator;
	size_t size = ::GetSerializeSize( messageCode, SER_NETWORK, PROTOCOL_VERSION );
	size += ::GetSerializeSize( _request->m_error, SER_NETWORK, PROTOCOL_VERSION );

	char * buffer = new char[size];

	CBufferAsStream stream(
				(char*)buffer
				, size
				, SER_NETWORK
				, CLIENT_VERSION);

	stream << messageCode;
	stream << _request->m_error;

	writeBuffer( buffer, size );
}

void
CLocalSocket::add( CProofTransactionAndStatusRequest const * _request )
{
	int messageCode = dims::CMessageKind::Transaction;

	size_t size = ::GetSerializeSize( messageCode, SER_NETWORK, PROTOCOL_VERSION );
	size += ::GetSerializeSize( _request->m_payApplicationData, SER_NETWORK, PROTOCOL_VERSION );

	char * buffer = new char[size];

	CBufferAsStream stream(
				(char*)buffer
				, size
				, SER_NETWORK
				, CLIENT_VERSION);

	//shitty
	stream << messageCode;
	stream << _request->m_payApplicationData;

	writeBuffer( buffer, size );
}

void
CLocalSocket::writeBuffer( char * _buffer, int _size )
{
	if ( !m_localSocket->isValid() )
		return;

	m_localSocket->write( QByteArray::fromRawData( _buffer, _size ) );
	m_localSocket->waitForBytesWritten ( -1 );
}

QLocalSocket *
CLocalSocket::getSocket() const
{
	return m_localSocket;
}

CLocalServer * CLocalServer::ms_instance = NULL;

CLocalServer*
CLocalServer::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CLocalServer();
	};
	return ms_instance;
}

bool
CLocalServer::getSocked( uintptr_t _ptr, CLocalSocket *& _localSocket ) const
{
	std::map< uintptr_t, CLocalSocket* >::const_iterator iterator = m_usedSockts.find( _ptr );

	if ( iterator == m_usedSockts.end() )
		return false;

	_localSocket = iterator->second;
	return true;
}

CLocalServer::CLocalServer()
{

	boost::filesystem::path path(dims::ServerName.toStdString());

	if ( boost::filesystem::exists( path ) )
		remove( path );

	if( m_server.listen( path.string().c_str() ))
	{
		connect(&m_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
	}
}
CLocalServer::~CLocalServer()
{
	typedef std::pair< uintptr_t, CLocalSocket* > Element;

	BOOST_FOREACH( Element const & element, m_usedSockts )
	{
		delete element.second;
	}
}

void
CLocalServer::newConnection()
{
	//Add all pending connection
	while (m_server.hasPendingConnections())
	{
		addToList( new CLocalSocket( m_server.nextPendingConnection() ) );
	}
}
void
CLocalServer::addToList( CLocalSocket* _socket )
{
	m_usedSockts.insert( std::make_pair( common::convertToInt( _socket->getSocket() ), _socket) );
	connect(_socket->getSocket(), SIGNAL( readyRead() ), this , SLOT( readSocket() ) );
	connect(_socket->getSocket(), SIGNAL( disconnected() ), this , SLOT( discardSocket() ) );
}

void CLocalServer::readSocket()
{
	assert( m_usedSockts.find( common::convertToInt( sender() ) ) != m_usedSockts.end() );

	m_usedSockts.find( common::convertToInt( sender() ) )->second->handleInput();
}

void CLocalServer::discardSocket()
{
	// how to handle this??
	QLocalSocket * socket = (QLocalSocket*)sender();

	assert( m_usedSockts.find( common::convertToInt( socket ) )!= m_usedSockts.end() );

// this  has to be  done  but not here in action handler most likely
//	delete m_usedSockts.find( common::convertToInt( socket ) )->second;

	m_usedSockts.erase( common::convertToInt( socket ) );
}

std::list< common::CClientBaseMedium *>
CLocalServer::provideConnection( common::CClientMediumFilter const & _mediumFilter )
{
	return _mediumFilter.getMediums( this );
}


}
