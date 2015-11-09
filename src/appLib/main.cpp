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

int main(int argc, char *argv[])
{
	dims::HardcodedSeed = "beufgsuyfguadfbnakjdfnhauifhbaisfhuashfusjaf";

	dims::AuthorId = "dXUT2rfnE8J7nqfa3TQHqPJnur6BX9YsFe";

	dims::Value = 100;

	dims::PossibleMonitors.push_back( "mK8nrDpWDoZ7Ez6x7aZwudwCcP3T9SEHix");

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
