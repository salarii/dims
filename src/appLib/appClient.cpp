// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/filesystem.hpp>

#include <QDataStream>
#include <QMessageBox>
#include <QMainWindow>
#include <QApplication>

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
	return conection->isOpen();
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
			if ( bytesWritten== (size_t)-1 )
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

bool CAppClient::checkApp( QApplication * _application, QMainWindow *_window )
{
	dims::CPaymentProcessing * paymentProcessing = dims::CPaymentProcessing::getInstance();
	if ( !paymentProcessing->isLicenseValid() )
	{
		QMessageBox::StandardButton reply;

		reply = QMessageBox::question( _window, "License missing", "Do you want to pay it now?",
									   QMessageBox::Yes|QMessageBox::No );

		connectServer();
		if (reply == QMessageBox::Yes)
		{
			while ( !isOpen() )
			{
				reply = QMessageBox::question( _window, "dims client not running", "Run dims client and press ok, or no to exit",
											   QMessageBox::Yes|QMessageBox::No);
				if (reply == QMessageBox::Yes)
				{
					continue;
				}
				else
				{
					_application->quit();
					return false;
				}
			}
			paymentProcessing->setEnableHook( boost::bind( &QMainWindow::setEnabled, _window, true ) );
			paymentProcessing->executeDialog( *this );
			return true;
		}
		else
		{
			_application->quit();
			return false;
		}
	}
	else
	{
		return true;
	}
}

}

