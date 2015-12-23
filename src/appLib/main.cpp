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

	dims::AuthorId = "dXUT2rfnE8J7nqfa3TQHqPJnur6BX9YsFe";  // enter  your address

	dims::Value = 7000;

	dims::PossibleMonitors.push_back( "mKzBm7EtrR1nZP3T5VNGGy9of2MPyPHUH4" );

	dims::PossibleMonitors.push_back( "mYh8ST9HsokJSZ9tr1qKrHN7ineJCDzcPt" );

	//dims::PossibleTrackers.push_back( "tJpviiWjg8FuvTrvXmPJM5btQQkneiLhTB");
	//dims::PossibleTrackers.push_back( "tJMhNj9DXPnRgDMDjYmCqvnjadQ4oRVfUt" );

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
