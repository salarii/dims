// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QApplication>
#include <QPushButton>
#include <QLabel>
#include <QMainWindow>

#include "testApplicaionWidgets.h"
#include "appClient.h"
#include "authorData.h"

#include "common/dimsParams.h"


// nasty!!
#ifdef WIN32

#include <QtPlugin>
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin)
#endif

int main(int argc, char *argv[])
{
  dims::HardcodedSeed = "beufgsuyfguadfbnakjdfnhauifhbaisfhuashfusjaf"; // random  pattern

	dims::AuthorId = "dW7fkLqxUN17n9KKxN5DDUBww5emdQq1NJ";  // enter  your address

	dims::Value = 7000;

	dims::PossibleMonitors.push_back( "mR3CmPx5U5jdyAHUPBWZFznrcYisfKkhNh" );

	dims::PossibleMonitors.push_back( "mWMBwVopNMDbmMQSMtKmut2g8HB2xEzUTy" );

	//dims::PossibleTrackers.push_back( "tWAyLrZL5GFshWc2wCF4JWUuymzfwXsL6c");
	//dims::PossibleTrackers.push_back( "tPNj1FsxrZdvyFmyuuHW26BhJQDvAZuMiK" );

	QApplication app (argc, argv);

	common::SelectDimsParams(CNetworkParams::TESTNET);

	QMainWindow *window = new QMainWindow();
	window->setWindowTitle( "Mass calculator" );
	window->setCentralWidget( new CMainWidget );

	window->show();
	window->setEnabled( false );

	dims::CAppClient appClient;

	if ( appClient.checkApp(&app,window) )
	{
		window->setEnabled( true );
	}

	return app.exec();
}
