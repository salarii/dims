#include "server.h"

using namespace std;

namespace self
{
class server_error : public std::runtime_error
{
public:
	explicit server_error(const std::string& str) : std::runtime_error(str) {}
};


CTcpServerConnection::CTcpServerConnection(Poco::Net::StreamSocket const & _serverConnection )
	: Poco::Net::TCPServerConnection( _serverConnection )
{
}

void
CTcpServerConnection::run()
{
	cout << "New connection from: " << socket().peerAddress().host().toString() <<  endl << flush;
	bool isOpen = true;
	Poco::Timespan timeOut(10,0);
	unsigned char incommingBuffer[1000];
	while( isOpen )
	{
		if (socket().poll(timeOut,Poco::Net::Socket::SELECT_READ) == false){
			throw server_error(std::string( "TIMEOUT!" ) + flush);
		}
		else{
			int nBytes = -1;

			try 
			{
				nBytes = socket().receiveBytes(incommingBuffer, sizeof(incommingBuffer));
			}
			catch (Poco::Exception& exc) {
				//Handle your network errors.
				throw server_error(std::string( "Network error:" ) + exc.displayText() );
				isOpen = false;
			}


			if (nBytes==0)
			{
				isOpen = false;
			}
			else
			{
				cout << "Receiving nBytes: " << nBytes << endl << flush;
			}
		}
	}
	cout << "Connection finished!" << endl << flush;
}
/*
int sendBytes(
const void * buffer,
int length,
int flags = 0
);

*/
}