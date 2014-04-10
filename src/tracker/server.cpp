#include "server.h"

#include "serialize.h"

#include "nodeMessage.h"

using namespace std;

namespace tracker
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
	while( isOpen )
	{
		if (socket().poll(timeOut,Poco::Net::Socket::SELECT_READ) == false){
			throw server_error(std::string( "TIMEOUT!" ) + flush);
		}
		else{
			int nBytes = -1;

			try 
			{
				nBytes = socket().receiveBytes( m_incommingBuffer.m_buffor, m_incommingBuffer.m_size );
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

void
CTcpServerConnection::writeSignature( CBufferAsStream & _stream )
{
	unsigned char signatureByte;
	for ( int i = 0 ; i < sizeof( MessageStartChars ); i++ )
	{
		_stream << signatureByte;

		if ( signatureByte != messageStart[ i ] )
			return false;
	}
	return true;
}


bool
CTcpServerConnection::checkSignature( CBufferAsStream const & _stream )
{
	unsigned char signatureByte;
	for ( int i = 0 ; i < sizeof( MessageStartChars ); i++ )
	{
		_stream >> signatureByte;

		if ( signatureByte != messageStart[ i ] )
			return false;
	}
	return true;

}

CBufferAsStream
CTcpServerConnection::createStream( CBuffor & _buffor )
{
	CBufferAsStream stream(
		  (char*)_buffor.m_buffor
		, _buffor.m_size
		, SER_DISK
		, CLIENT_VERSION);

	return stream;
}

bool
CTcpServerConnection::handleIncommingBuffor()
{
	MessageStartChars const & messageStart = m_networkParams->MessageStart();

	CBufferAsStream inStream = createStream( m_incommingBuffer );
	CBufferAsStream outStream = createStream( m_outgoingBuffer );

	if ( !checkSignature( stream ) )
		return false;

	writeSignature( outStream );

	while( !stream.eof() )
	{
		int messageType;

		stream >> messageType;

		switch( (CClientMessageType::Enum)messageType )
		{
			case CClientMessageType::Transaction:

				CTransaction transaction;
				stream >> transaction;
				outStream << transaction.GetHash();
				m_validationManager->serviceTransaction( transaction );
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
/*
int sendBytes(
const void * buffer,
int length,
int flags = 0
);

*/
}