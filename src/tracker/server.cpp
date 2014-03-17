#include "server.h"

#include "serialize.h"

#include "nodeMessage.h"

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
			/* handle  incomming  connections */
			
			/* answare  to  incomming  connections */
		}
	}
	cout << "Connection finished!" << endl << flush;
}

bool
CTcpServerConnection::handleIncommingBuffor( unsigned char* _buffor, unsigned int _size )
{
// header match
	MessageStartChars const & messageStart = m_networkParams->MessageStart();

	CBufferAsStream stream(
	 (char*)_buffor
	, _size
	, SER_DISK
	, CLIENT_VERSION);

	unsigned char signatureByte;
	for ( int i = 0 ; i < sizeof( MessageStartChars ); i++ )
	{
		stream >> signatureByte;

		if ( signatureByte != messageStart[ i ] )
			return false;
	}

	while( !stream.eof() )
	{
		int messageType;

		stream >> messageType;

		switch( (CClientMessageType::Enum)messageType )
		{
			case CClientMessageType::Transaction:

				CTransactionMessage transactionMessage;
				stream >> transactionMessage.m_transaction;

				break;
			case CClientMessageType::TrackerInfoReq:
				break;
			case CClientMessageType::MonitorInfoReq:
				break;
			case CClientMessageType::TransactionInfoReq:
				break;
			default:
				return false;
		}
	}

}
CTransactionMessage
CTrackerInfoReq
CMonitorInfoReq
CTransactionInfoReq
/*
int sendBytes(
const void * buffer,
int length,
int flags = 0
);

*/
}