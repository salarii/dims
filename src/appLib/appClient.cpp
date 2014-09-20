// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "appClient.h"
#include <QDataStream>

namespace dims
{

CAppClient::CAppClient( QString const & _dimsClient )
{
	conection = new QLocalSocket(this);

	connect(conection,SIGNAL(readyRead()),this,SLOT(readSocket()));

	connect(conection,SIGNAL(disconnected()),this,SLOT(discardSocket()));

	conection->connectToServer( _dimsClient );
}
CAppClient::~CAppClient()
{
	if(conection)
		conection->close();
}
void CAppClient::readSocket()
{
	QByteArray block = conection->readAll();
	QDataStream in(&block, QIODevice::ReadOnly);
	in.setVersion(QDataStream::Qt_4_0);
	while (!in.atEnd())
	{
		QString receiveString;
		in >> receiveString;
		receiveString.prepend(QString("%1 :: ").arg(conection->socketDescriptor()));
	}
}

void CAppClient::discardSocket()
{
	conection->deleteLater();
	conection = 0;
}

void CAppClient::send()
{
	if(conection)
	{
		if(conection->isOpen())
		{
			QByteArray block;
			QDataStream out(&block, QIODevice::WriteOnly);
			out << QString( "pixi" );
			conection->write(block);
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





