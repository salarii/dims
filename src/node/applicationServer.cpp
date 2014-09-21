// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/filesystem.hpp>

#include "applicationServer.h"
#include "common/medium.h"
#include "common/support.h"

#include "appLib/messageType.h"

namespace client
{

CLocalSocket::CLocalSocket( QLocalSocket * _localSocket )
	:m_localSocket( _localSocket )
{
}

CLocalSocket::~CLocalSocket()
{
	size_t size = m_localSocket->bytesAvailable ();
	m_localSocket->close();
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

}

bool
CLocalSocket::flush()
{

}

void
CLocalSocket::clearResponses()
{

}

bool
CLocalSocket::getResponse( std::vector< NodeResponses > & _requestResponse ) const
{

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

CLocalServer::CLocalServer()
{

#ifndef WIN32

	boost::filesystem::path path(dims::ServerName.toStdString());

	if ( boost::filesystem::exists( path ) )
		remove( path );

#endif
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

	delete m_usedSockts.find( common::convertToInt( socket ) )->second;

	m_usedSockts.erase( common::convertToInt( socket ) );
}


}
