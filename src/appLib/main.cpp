#include "appClient.h"
#include "paymentProcessing.h"

#include <QtCore/QCoreApplication>

// check file  validity

// start server proceed

namespace  dims
{
std::vector< unsigned char > const HardcodedSeed;

std::vector< std::string > const PossibleMonitors;

std::vector< std::string > const PossibleTrackers;

CKeyID authorId;

int Value;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	dims::CAppClient appClient;
	appClient.connectServer();

	dims::CPaymentProcessing paymentProcessing;

	if ( !appClient.isOpen() )
			assert(!"don't like this");

	paymentProcessing.executeDialog( appClient );

	return a.exec();
}
