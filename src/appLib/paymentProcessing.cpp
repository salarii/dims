// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>

#include "paymentData.h"
#include "paymentProcessing.h"

#include "util.h"

namespace dims
{

extern CKeyID authorId;

extern int Value;

unsigned int const LicenseFileSize = 1024 * 2;
bool
CPaymentProcessing::licenseExist()
{
	boost::filesystem::path path( ".license" );

	return boost::filesystem::exists(path);
}

FILE*
CPaymentProcessing::openLicenseFile()
{
	boost::filesystem::path path = ".license";

	FILE* file = fopen(path.string().c_str(), "rb+");
	if (!file )
		file = fopen(path.string().c_str(), "wb+");
	if (!file) {
		return NULL;
	}

	return file;
}

bool
CPaymentProcessing::readLicenseFileData( CLicenseData & _licenseData )
{
	if ( !licenseExist() )
		return false;

	FILE* license = openLicenseFile();

	if ( license == NULL )
		return false;

	CBufferedFile licenseStream( license, 2*LicenseFileSize, LicenseFileSize, SER_NETWORK, CLIENT_VERSION);

	licenseStream >> _licenseData;

	return true;
}

bool
CPaymentProcessing::verifyData( CLicenseData const & _licenseData )
{
	uint256 hash = Hash( _licenseData.m_privateKey.begin(), _licenseData.m_privateKey.end() );

	HardwareKeys hardwareKeys = createKeys();

	if(
			! hardwareKeys.m_macBased.GetPubKey().Verify( hash, _licenseData.m_macSignature )
		&& ! hardwareKeys.m_procBased.GetPubKey().Verify( hash, _licenseData.m_procSignature )
		&& ! hardwareKeys.m_volumeBased.GetPubKey().Verify( hash, _licenseData.m_volumeSignature )
			)
		return false;

	if ( !analyseOutput( _licenseData.m_trasaction, authorId, Value ) )
		return false;
	CKey key;
	key.SetPrivKey(_licenseData.m_privateKey, false);
	if ( !analyseInput( _licenseData.m_trasaction, key.GetPubKey().GetID() ) )
		return false;

	return true;
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
					if ( _authorId == Hash160( *it ) && txout.nValue  >= _value )
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
	key.Set( &seed[0], &seed[ seed.size()-1 ], false );

	m_licenseData.m_privateKey = key.GetPrivKey();
}

CKey
CPaymentProcessing::createHardwareKey( std::vector< unsigned char > const & _hardwareData )
{
	std::vector< unsigned char > seed = HardcodedSeed;

	size_t size = _hardwareData.size();

	seed.resize( 32 -size, 0 );

	seed.insert( seed.end(), _hardwareData.begin(), _hardwareData.end() );

	CKey key;

	key.Set( &seed[0], &seed[ seed.size()-1 ], false );

	return key;
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
