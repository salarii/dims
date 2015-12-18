// Copyright (c) 2014-2015 DiMS dev-team
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PAYMENT_PROCESSING_H
#define PAYMENT_PROCESSING_H

#include "key.h"
#include "hardwareInfo.h"
#include "paymentData.h"
#include "authorData.h"
#include <boost/signals2.hpp>

// in theory slight changes in hardware are allowed
namespace dims
{
class CAppClient;

//  singleton ??
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
	void createOrUpdateLicenseFile( CLicenseData const & _licenseData );

	CKey getRandomKey() const;

	void executeDialog( CAppClient & _appClient );

	bool isLicenseValid();

	static CPaymentProcessing* getInstance();

	bool serviceMessage( char * _buffer, size_t _size );

	void setEnableHook( boost::signals2::slot< void() > const & _slot )
	{
		m_enableHook.connect( _slot );
	}
private:
    CPaymentProcessing();

	bool readLicenseFileData();

	bool saveLicenseFileData( CLicenseData const & _licenseData );

	FILE* openLicenseFile();

	bool licenseExist();

	template < class Message >
	char * createMessage( Message const & _message, size_t & _size );

	std::vector<CKeyID> convertToKeyId( std::vector< std::string > const & _keys ) const;

	bool verifyData( CLicenseData const & _licenseData );

	bool analyseOutput( CTransaction const & _tx, CKeyID const & _authorId, int _value );

	bool analyseInput( CTransaction const & _tx, CKeyID const & _thisInstance );

	void generateKey();

	HardwareKeys createKeys();

	void signPrivateKey();

	CKey createHardwareKey( std::vector< unsigned char > const & _hardwareData );

private:
	boost::signals2::signal<void ()> m_enableHook;

	CLicenseData m_licenseData;

	static CPaymentProcessing * ms_instance;
};


}

#endif // PAYMENT_PROCESSING_H
