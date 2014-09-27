// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "appClient.h"
#include "paymentProcessing.h"
#include "common/dimsParams.h"

#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>

namespace  dims
{
std::vector< unsigned char > const HardcodedSeed;

std::vector< std::string > const PossibleMonitors;

std::vector< std::string > const PossibleTrackers = boost::assign::list_of( "tNuH46LxbgdT47qtUHzvmHDpJQyAydYZGn" );

std::string const AuthorId = "dKqwKfMdUcxipQ82NzP1NoZ3AtTEUsGMKV";

int Value = 10000;
}

int main(int argc, char *argv[])
{
	QApplication app (argc, argv);

	if ( argc == 2 )
	{
		if ( std::string( argv[1] ) == std::string( "-testnet" ) )
			common::SelectRatcoinParams(CNetworkParams::TESTNET);
	}

	QPushButton button ("application paid");

	dims::CPaymentProcessing * paymentProcessing = dims::CPaymentProcessing::getInstance();
	button.show();
	if ( !paymentProcessing->isLicenseValid() )
	{
		dims::CAppClient appClient;

		QMessageBox::StandardButton reply;


		reply = QMessageBox::question( &button, "License missing", "Do you want to pay it now?",
									  QMessageBox::Yes|QMessageBox::No );

		appClient.connectServer();
		if (reply == QMessageBox::Yes)
		{
			while ( !appClient.isOpen() )
			{
				reply = QMessageBox::question( &button, "dims client not running", "Run dims client and press ok, or no to exit",
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
