// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "appClient.h"
#include "paymentProcessing.h"
#include "common/dimsParams.h"

#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QMainWindow>

#include "testApplicaionWidgets.h"

namespace  dims
{

std::string const HardcodedSeed = "beufgsuyfguadfbnakjdfnhauifhbaisfhuashfusjaf";

std::vector< std::string > const PossibleMonitors;

std::vector< std::string > const PossibleTrackers = boost::assign::list_of( "tJMhNj9DXPnRgDMDjYmCqvnjadQ4oRVfUt" );

std::string const AuthorId = "dKqwKfMdUcxipQ82NzP1NoZ3AtTEUsGMKV";

int Value = 10000;
}

int main(int argc, char *argv[])
{
	QApplication app (argc, argv);

	QMainWindow *window = new QMainWindow();
	window->setWindowTitle( "Mass calculator" );
	window->setCentralWidget( new CMainWidget );

	window->show();

	if ( argc == 2 )
	{
		if ( std::string( argv[1] ) == std::string( "-testnet" ) )
			common::SelectRatcoinParams(CNetworkParams::TESTNET);
	}

	dims::CPaymentProcessing * paymentProcessing = dims::CPaymentProcessing::getInstance();

	dims::CAppClient appClient;
	if ( !paymentProcessing->isLicenseValid() )
	{
		QMessageBox::StandardButton reply;

		reply = QMessageBox::question( window, "License missing", "Do you want to pay it now?",
									  QMessageBox::Yes|QMessageBox::No );

		appClient.connectServer();
		if (reply == QMessageBox::Yes)
		{
			while ( !appClient.isOpen() )
			{
				reply = QMessageBox::question( window, "dims client not running", "Run dims client and press ok, or no to exit",
											  QMessageBox::Yes|QMessageBox::No);
				if (reply == QMessageBox::Yes)
				{
					continue;
				}
				else
				{
					app.quit();
					return 0;
				}
			}

			paymentProcessing->executeDialog( appClient );
		}
		else
		{
			app.quit();
			return 0;
		}
	}

	return app.exec();
}
