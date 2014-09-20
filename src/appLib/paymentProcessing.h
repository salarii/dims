// Copyright (c) 2014 Dims dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PAYMENT_PROCESSING_H
#define PAYMENT_PROCESSING_H

#include "key.h"
#include "hardwareInfo.h"

// in theory slight changes in hardware are allowed
namespace dims
{

extern std::vector< unsigned char > const HardcodedSeed;

class CPaymentProcessing
{
public:
	struct HardwareKeys
	{
		CKey m_macBased;
		CKey m_procBased;
		CKey m_volumeBased;
	};
public:
	CPaymentProcessing(){};

	bool licenseExist();

	FILE* openLicenseFile();

	bool readLicenseFileData( CLicenseData & _licenseData );

	void createOrUpdateLicenseFile( CLicenseData const & _licenseData );

	CKey getRandomKey() const;
private:
	bool verifyData( CLicenseData const & _licenseData );

	bool analyseOutput( CTransaction const & _tx, CKeyID const & _authorId, int _value );

	bool analyseInput( CTransaction const & _tx, CKeyID const & _thisInstance );

	void generateKey();

	HardwareKeys createKeys();

	void signPrivateKey();

	CKey createHardwareKey( std::vector< unsigned char > const & _hardwareData );
private:
	CLicenseData m_licenseData;
};


}

#endif // PAYMENT_PROCESSING_H
