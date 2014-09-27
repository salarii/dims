// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/filesystem.hpp>

#include <QDataStream>

#include "appClient.h"
#include "messageType.h"
#include "util.h"
#include "paymentProcessing.h"

namespace dims
{

CAppClient::CAppClient()
{
	conection = new QLocalSocket(this);

	connect(conection,SIGNAL(readyRead()),this,SLOT(readSocket()));

	connect(conection,SIGNAL(disconnected()),this,SLOT(discardSocket()));

	connect(conection,SIGNAL(error(QLocalSocket::LocalSocketError)),this,SLOT(what(QLocalSocket::LocalSocketError)));
}

void
CAppClient::connectServer()
{
	conection->connectToServer( ServerName );
}


bool
CAppClient::isOpen()
{
	conection->isOpen();
}

CAppClient::~CAppClient()
{
	if(conection)
		conection->close();
}
void CAppClient::readSocket()
{
	QByteArray block = conection->readAll();

	CPaymentProcessing::getInstance()->serviceMessage( block.data(), block.size() );
}

void
CAppClient::what(QLocalSocket::LocalSocketError _error )
{
}

void CAppClient::discardSocket()
{
	conection->deleteLater();
	conection = 0;
}

void CAppClient::send( QByteArray const & _message )
{
	if(conection)
	{
		if(conection->isOpen())
		{
			size_t bytesWritten = conection->write(_message );
			if ( bytesWritten== -1 )
				assert(!"couldn't send anything");
		}
		else
		{
			//"Connection isn't open!"
		}
	}
	else
	{
		//"Not connected!");
	}
}

}





