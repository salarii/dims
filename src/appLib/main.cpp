#include "appClient.h"
#include "paymentProcessing.h"
#include "common/dimsParams.h"

#include <QtCore/QCoreApplication>

namespace  dims
{
std::vector< unsigned char > const HardcodedSeed;

std::vector< std::string > const PossibleMonitors;

std::vector< std::string > const PossibleTrackers = boost::assign::list_of( "tC4NgiF8rG3WTXzdbG5B3JXG6dj8G3prSu" );

CKeyID authorId;

int Value = 1000;
}

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	if ( argc == 2 )
	{
		if ( std::string( argv[1] ) == std::string( "-testnet" ) )
			common::SelectRatcoinParams(CNetworkParams::TESTNET);
	}

	dims::CAppClient appClient;
	appClient.connectServer();

	dims::CPaymentProcessing paymentProcessing;

	if ( !appClient.isOpen() )
			assert(!"don't like this");

	if ( !paymentProcessing.isLicenseValid() )
		paymentProcessing.executeDialog( appClient );

	return a.exec();
}
