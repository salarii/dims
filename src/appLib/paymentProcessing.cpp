// Copyright (c) 2014-2015 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include <QApplication>
#include <QMessageBox>

#include "paymentProcessing.h"
#include "messageType.h"
#include "appClient.h"

#include "util.h"
#include "base58.h"

#include "node/clientRequests.h"

namespace dims
{

std::string const LicenseFilePath = ".license";

CPaymentProcessing * CPaymentProcessing::ms_instance = NULL;

CPaymentProcessing*
CPaymentProcessing::getInstance()
{
	if ( !ms_instance )
	{
		ms_instance = new CPaymentProcessing();
	};
	return ms_instance;
}

unsigned int const LicenseFileSize = 1024 * 2;
bool
CPaymentProcessing::licenseExist()
{
	boost::filesystem::path path( LicenseFilePath );

	return boost::filesystem::exists(path);
}

FILE*
CPaymentProcessing::openLicenseFile()
{
	boost::filesystem::path path = LicenseFilePath;

	FILE* file = fopen(path.string().c_str(), "rb+");
	if (!file )
		file = fopen(path.string().c_str(), "wb+");
	if (!file) {
		return NULL;
	}

	return file;
}

bool
CPaymentProcessing::readLicenseFileData()
{
	if ( !licenseExist() )
		return false;

	FILE* license = openLicenseFile();

	if ( license == NULL )
		return false;

	CBufferedFile licenseStream( license, 2*LicenseFileSize, LicenseFileSize, SER_NETWORK, CLIENT_VERSION);

	licenseStream >> m_licenseData;

	return true;
}

bool
CPaymentProcessing::saveLicenseFileData( CLicenseData const & _licenseData )
{
	FILE* license = openLicenseFile();

	if ( license == NULL )
		return false;

	CAutoFile licenseFile( license, SER_NETWORK, CLIENT_VERSION);

	licenseFile << m_licenseData;

	return true;
}

bool
CPaymentProcessing::verifyData( CLicenseData const & _licenseData )
{
	CKey key;
	key.SetPrivKey( m_licenseData.m_privateKey, false);
	uint256 hash = Hash( key.begin(), key.end() );

	HardwareKeys hardwareKeys = createKeys();

	if(
			! hardwareKeys.m_macBased.GetPubKey().Verify( hash, _licenseData.m_macSignature )
		&& ! hardwareKeys.m_procBased.GetPubKey().Verify( hash, _licenseData.m_procSignature )
		&& ! hardwareKeys.m_volumeBased.GetPubKey().Verify( hash, _licenseData.m_volumeSignature )
			)
		return false;

	CMnemonicAddress authorAddress;
	authorAddress.SetString( AuthorId );

	CKeyID authorKeyId;
	if ( !authorAddress.GetKeyID( authorKeyId ) )
		return false;

	if ( !analyseOutput( _licenseData.m_trasaction, authorKeyId, Value ) )
		return false;

	if ( !analyseInput( _licenseData.m_trasaction, key.GetPubKey().GetID() ) )
		return false;

	if ( !PossibleMonitors.empty() )
	{
		CNodeAddress nodeAddress;
		nodeAddress.Set( _licenseData.m_monitorPubKey.GetID(), common::NodePrefix::Monitor );
		std::vector< std::string >::const_iterator iterator = std::find( PossibleMonitors.begin(), PossibleMonitors.end(), nodeAddress.ToString() );
		if ( iterator != PossibleMonitors.end() )
		{
			BOOST_FOREACH( common::CNodeInfo const & nodeInfo, _licenseData.m_monitorData.m_monitors )
			{
				if ( nodeInfo.m_key == _licenseData.m_trackerPubKey )
				{
					if ( _licenseData.m_trackerPubKey.Verify( _licenseData.m_trasaction.GetHash(), _licenseData.m_transactionStatusSignature ) )
						return true;
				}
			}
		}
	}

	if ( !PossibleTrackers.empty() )
	{
		CNodeAddress nodeAddress;
		nodeAddress.Set( _licenseData.m_trackerPubKey.GetID(), common::NodePrefix::Tracker );
		std::vector< std::string >::const_iterator iterator = std::find( PossibleTrackers.begin(), PossibleTrackers.end(), nodeAddress.ToString() );
		if ( iterator != PossibleTrackers.end() )
		{
			if ( _licenseData.m_trackerPubKey.Verify( _licenseData.m_trasaction.GetHash(), _licenseData.m_transactionStatusSignature ) )
				return true;
		}
	}

	return false;
}

bool
CPaymentProcessing::analyseOutput( CTransaction const & _tx, CKeyID const & _authorId, int _value )
{
	for (unsigned int i = 0; i < _tx.vout.size(); i++)
	{
		const CTxOut& txout = _tx.vout[i];

		opcodetype opcode;

		std::vector<unsigned char> data;

		CScript::const_iterator pc = txout.scriptPubKey.begin();
	//sanity check
		while( pc != txout.scriptPubKey.end() )
		{
			if (!txout.scriptPubKey.GetOp(pc, opcode, data))
				return false;
		}
		txnouttype type;

		std::vector< std:: vector<unsigned char> > vSolutions;
		if (Solver(txout.scriptPubKey, type, vSolutions) &&
			(type == TX_PUBKEY || type == TX_PUBKEYHASH))
		{
			std::vector<std::vector<unsigned char> >::iterator it = vSolutions.begin();

			while( it != vSolutions.end() )
			{
				if ( type == TX_PUBKEY )
				{
					// impossible  to be here ??
					if ( _authorId == Hash160( *it ) && txout.nValue  >= _value )
						return true;
				}
				else
				{
					if ( _authorId == uint160( *it ) && txout.nValue  >= _value )
						return true;
				}
				it++;
			}
		}

	}
	return false;
}

bool
CPaymentProcessing::analyseInput( CTransaction const & _tx, CKeyID const & _thisInstance )
{
	for (unsigned int i = 0; i < _tx.vin.size(); i++)
	{
		const CTxIn& txin = _tx.vin[i];

		CScript::const_iterator pc = txin.scriptSig.begin();

		opcodetype opcode;

		std::vector<unsigned char> data;

		while( pc < txin.scriptSig.end() )
		{
			if (!txin.scriptSig.GetOp(pc, opcode, data))
				return false;

			if ( data.size() == 33 || data.size() == 65 )
			{
				if ( CPubKey( data ).GetID() == _thisInstance )
					return true;
			}
		}
	}
	return false;
}

void
CPaymentProcessing::generateKey()
{
	seed_insecure_rand();

	std::vector< unsigned char > seed;
	seed.resize(32);

	BOOST_FOREACH( unsigned char & element, seed )
	{
		element = insecure_rand();
	}

	CKey key;
	key.Set( &seed[0], &seed[ seed.size() ], false );

	m_licenseData.m_privateKey = key.GetPrivKey();
}

CKey
CPaymentProcessing::createHardwareKey( std::vector< unsigned char > const & _hardwareData )
{
	std::vector< unsigned char > seed;

	std::copy( HardcodedSeed.begin(), HardcodedSeed.end(), std::back_inserter( seed ));

	size_t size = _hardwareData.size();

	seed.resize( 32 -size, 0 );

	seed.insert( seed.end(), _hardwareData.begin(), _hardwareData.end() );

	CKey key;

	key.Set( &seed[0], &seed[ seed.size() ], false );

	return key;
}

void
CPaymentProcessing::executeDialog( CAppClient & _appClient )
{
	generateKey();

	CExpectationMessage expectationMessage;

	expectationMessage.m_privateKey = m_licenseData.m_privateKey;

	expectationMessage.m_monitors = convertToKeyId( PossibleMonitors );

	expectationMessage.m_trackers = convertToKeyId( PossibleTrackers );
	CMnemonicAddress authorAddress;
	authorAddress.SetString( AuthorId );

	CKeyID authorKeyId;
	if ( !authorAddress.GetKeyID( authorKeyId ) )
	{
		assert( !"shouldn't be here" );
		return;
	}

	expectationMessage.m_targetId = authorKeyId;

	expectationMessage.m_value = Value;

	CPackedMessage< CExpectationMessage > message( CMessageKind::Expectations, expectationMessage );

	size_t size;
	char * messageBuffer = createMessage( message, size );

	_appClient.send( QByteArray::fromRawData( messageBuffer, size ) );

	delete messageBuffer;

}

bool
CPaymentProcessing::isLicenseValid()
{
	if ( !licenseExist() )
		return false;

	if ( !readLicenseFileData() )
		return false;

	if ( !verifyData( m_licenseData ) )
		return false;

	return true;
}

template < class Message >
char *
CPaymentProcessing::createMessage( Message const & _message, size_t & _size )
{
	size_t size = ::GetSerializeSize(_message, SER_NETWORK, PROTOCOL_VERSION);

	char * buffer = new char[size];

	CBufferAsStream stream(
		  (char*)buffer
		, size
		, SER_NETWORK
		, CLIENT_VERSION);

	stream << _message;

	_size = size;
	return buffer;
}

bool
CPaymentProcessing::serviceMessage( char * _buffer, size_t _size )
{
	CBufferAsStream stream(
		  (char*)_buffer
		, _size
		, SER_NETWORK
		, CLIENT_VERSION);

	int kind;

	stream >> kind;

	if ( kind == CMessageKind::Transaction )
	{

		common::CPayApplicationData proofTransactionAndStatus;

		stream >> proofTransactionAndStatus;

		m_licenseData.m_trasaction = proofTransactionAndStatus.m_trasaction;
		m_licenseData.m_transactionStatusSignature = proofTransactionAndStatus.m_transactionStatusSignature;
		m_licenseData.m_trackerPubKey = proofTransactionAndStatus.m_servicingTracker;
		m_licenseData.m_monitorData = proofTransactionAndStatus.m_monitorData;
		m_licenseData.m_monitorPubKey = proofTransactionAndStatus.m_servicingMonitor;

		signPrivateKey();
		verifyData( m_licenseData );

		saveLicenseFileData( m_licenseData );
	}
	else if ( kind == CMessageKind::ErrorIndicator )
	{
		int m_error;
		stream >> m_error;
		QMessageBox::question( 0, "error", "Error on client side press Ok to exit", QMessageBox::Ok);
		QApplication::quit();
	}
	else
	{
		return false;
	}
	return true;
}

std::vector<CKeyID>
CPaymentProcessing::convertToKeyId( std::vector< std::string > const & _keys ) const
{
	std::vector<CKeyID> keyIds;
	BOOST_FOREACH( std::string const & stringKey,_keys )
	{
		CNodeAddress nodeAddress(stringKey);

		CKeyID keyId;
		if ( nodeAddress.GetKeyID(keyId) )
			keyIds.push_back( keyId );
	}

	return keyIds;
}

CPaymentProcessing::HardwareKeys
CPaymentProcessing::createKeys()
{
	HardwareKeys hardwareKeys;
	CHardwareNumbers hardwareNumbers = getHardwareNumbers();

	hardwareKeys.m_macBased = createHardwareKey( hardwareNumbers.m_macHash );
	hardwareKeys.m_procBased = createHardwareKey( hardwareNumbers.m_volumeHash );
	hardwareKeys.m_volumeBased = createHardwareKey( hardwareNumbers.m_cpuHash );

	return hardwareKeys;
}

void
CPaymentProcessing::signPrivateKey()
{
	CKey key;
	key.SetPrivKey( m_licenseData.m_privateKey, false);
	uint256 hash = Hash( key.begin(), key.end() );

	HardwareKeys hardwareKeys = createKeys();

	hardwareKeys.m_macBased.Sign( hash, m_licenseData.m_macSignature );
	hardwareKeys.m_procBased.Sign( hash, m_licenseData.m_procSignature );
	hardwareKeys.m_volumeBased.Sign( hash, m_licenseData.m_volumeSignature );
}

}
